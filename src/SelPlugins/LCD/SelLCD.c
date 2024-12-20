/***
 * Display messages on an LCD textual screen (like 1602 one)
 *
 * Based on https://fr.wikipedia.org/wiki/HD44780 
 * and inspired by BitBank https://github.com/bitbank2/LCD1602

@classmod SelOLED

 * 06/09/2024 LF : First version
 */

#include <Selene/SelPlug-in/SelLCD.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <assert.h>

#if LUA_VERSION_NUM == 501
#	define lua_rawlen lua_objlen
#endif

static struct SelLCD selLCD;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

/* I2C expender bits usage :
 *
 * 0 : RS (0 = command, 1 = data)
 * 1 : Read (0) / Write (1)
 * 2 : Enable
 * 3 : Backlight
 */
#define RS_CMD 0
#define RS_DATA 0x01
#define RW_R 0
#define RW_W 0x02
#define ENABLE 0x04
#define BACKLIGHT 0x08

static struct LCDscreen *checkSelLCD(lua_State *L){
	void *r = selLua->testudata(L, 1, "SelLCD");
	luaL_argcheck(L, r != NULL, 1, "'SelLCD' expected");

	return (struct LCDscreen *)r;
}

static void lcdc_SendQuarter(struct LCDscreen *lcd, uint8_t b){
/* Send the provided quarter */

	write(lcd->bus, &b, 1);	/* Present the data on the gpio */
	usleep(selLCD.clock_pulse);

	b |= ENABLE;		/* Rise 'E' */
	write(lcd->bus, &b, 1);
	usleep(selLCD.clock_pulse);

	b &= ~(ENABLE);		/* Lower 'E' */
	write(lcd->bus, &b, 1);
	usleep(selLCD.clock_process);
}

static void lcdc_SendCmd(struct LCDscreen *lcd, uint8_t dt){
/** 
 * @brief Send a command to the LCD controller
 *
 * @function SendCmd
 * @tparam uint8_t command to send
 */
	uint8_t t = RS_CMD;	/* It's a Command */
	t |= lcd->backlight ? BACKLIGHT : 0;		/* Is the backlight on ? */

		/* Most significant quarter first */
	t |= dt & 0xf0;
	selLCD.SendQuarter(lcd, t);

	t &= 0x0f;	/* Keep only control bits */
	t |= dt << 4;	/* send less significant quarter */
	selLCD.SendQuarter(lcd, t);
}

static void lcdc_SendData(struct LCDscreen *lcd, uint8_t dt){
/** 
 * @brief Send a data to the LCD controller
 *
 * @function SendData
 * @tparam uint8_t data to send
 */
	uint8_t t = RS_DATA;	/* It's a Command */
	t |= lcd->backlight ? BACKLIGHT : 0;		/* Is the backlight on ? */

		/* Most significant quarter first */
	t |= dt & 0xf0;
	selLCD.SendQuarter(lcd, t);

	t &= 0x0f;	/* Keep only control bits */
	t |= dt << 4;	/* send less significant quarter */
	selLCD.SendQuarter(lcd, t);
}

static bool lcdc_Init(struct LCDscreen *lcd, uint16_t bus_number, uint8_t address, bool twolines, bool y11){
/** 
 * @brief Initialize connection to the screen
 *
 * @function Init
 * @param screen point to the screen handle
 * @tparam uint16_t I2C bus number
 * @tparam uint8_t Screen I2C address
 * @tparam boolean true if the screen has 2 lines 
 * @tparam boolean true if the screen is 11 pixel hight
 * @treturn boolean false if we faced a technical error
 */
	char sbus[16];
	sprintf(sbus, "/dev/i2c-%u", bus_number);

	if((lcd->bus = open(sbus, O_RDWR)) < 0)
		return false;

	if(ioctl(lcd->bus, I2C_SLAVE, address) < 0)
		return false;

	if(i2c_smbus_write_quick(lcd->bus, I2C_SMBUS_WRITE) < 0){
		close(lcd->bus);
		return false;
	}
		
		/* Initializing 
		 * SET + 4 bits mode
		 * We're sending the upper quarter first so 0x02 is really 0x20.
		 */
	selLCD.SendCmd(lcd, 0x02);

		/* Now sending the full configuration */
	selLCD.SendCmd(lcd, 0x02 | (twolines ? 0x08 : 0) | (y11 ? 0x04 : 0));
	
	return true;
}

static int lcdl_Init(lua_State *L){
	uint16_t nbus = luaL_checkinteger(L, 1);
	uint8_t addr = luaL_checkinteger(L, 2);
	bool twolines = lua_toboolean(L, 3);
	bool y11 = lua_toboolean(L, 4);

	struct LCDscreen *lcd = (struct LCDscreen *)lua_newuserdata(L, sizeof(struct LCDscreen));
	assert(lcd);

	luaL_getmetatable(L, "SelLCD");
	lua_setmetatable(L, -2);

	if(!selLCD.Init(lcd, nbus, addr, twolines, y11)){
		lua_pop(L, 1);
		return 0;
	}

	return 1;
}

static int lcdl_SetTiming(lua_State *L){
/**
 * @brief Set LCD timming
 *
 *	It's an optimisation function. Default value are safe, yours are ...
 * on your hand only.
 *
 * @function SetTiming
 * @param E timing in microsecond
 * @param process timing in microsecond
 */
	selLCD.clock_pulse = luaL_checkinteger(L, 1);
	selLCD.clock_process = luaL_checkinteger(L, 2);

	return 0;
}

static void lcdc_Shutdown(struct LCDscreen *lcd){
/**
 * @brief Turn off the screen
 *
 * @function Shutdown
 * @param screen point to the screen handle
 */
	selLCD.DisplayCtl(lcd, false, false, false);
	close(lcd->bus);
	lcd->bus = -1;
}

static int lcdl_Shutdown(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);

	selLCD.Shutdown(lcd);

	return 0;
}

static void lcdc_Backlight(struct LCDscreen *lcd, bool bl){
/** 
 * @brief Turn backlight on or off (for next command)
 *
 * @function backlight
 * @param screen point to the screen handle
 * @tparam boolean status of the backlight
 */
	lcd->backlight = bl;
}

static int lcdl_Backlight(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	bool bl = lua_toboolean(L, 2);

	selLCD.Backlight(lcd, bl);

	return 0;
}

static void lcdc_DisplayCtl(struct LCDscreen *lcd, bool screen, bool cursor, bool blink){
/** 
 * @brief Display control
 *
 * @function DisplayCtl
 * @param screen point to the screen handle
 * @tparam boolean is the screen on ?
 * @tparam boolean is the _ cursor on ?
 * @tparam boolean is the block cursor on ?
 */
	uint8_t t = 0x08;	/* Display control */
	t |= screen ? 0x04:0x00;
	t |= cursor ? 0x02:0x00;
	t |= blink ? 0x01:0x00;

	selLCD.SendCmd(lcd, t);
}

static int lcdl_DisplayCtl(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	bool screen = lua_toboolean(L, 2);
	bool cursor = lua_toboolean(L, 3);
	bool blink = lua_toboolean(L, 4);

	selLCD.DisplayCtl(lcd, screen, cursor, blink);

	return 0;
}

static void lcdc_EntryCtl(struct LCDscreen *lcd, bool inc, bool shift){
/** 
 * @brief Entry control
 *
 * @function EntryCtl
 *
 * @param screen point to the screen handle
 * @tparam boolean increment the cursor when a character is sent
 * @tparam boolean shift the screen if the cursor leaves it
 */
	uint8_t t = 0x04;		/* Entry control */
	t |= inc ? 0x02 : 0x00;
	t |= shift ? 0x01 : 0x00;

	selLCD.SendCmd(lcd, t);
}

static int lcdl_EntryCtl(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	bool inc = lua_toboolean(L, 2);
	bool shift = lua_toboolean(L, 3);

	selLCD.EntryCtl(lcd, inc, shift);

	return 0;
}

static void lcdc_Clear(struct LCDscreen *lcd){
/** 
 * @brief Clear the screen
 *
 * @function Clear
 *
 * @param screen point to the screen handle
 */
	selLCD.SendCmd(lcd, 0x01);
}

static int lcdl_Clear(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);

	selLCD.Clear(lcd);

	return 0;
}

static void lcdc_Home(struct LCDscreen *lcd){
/** 
 * @brief Places cursor at up-left position
 *
 * @function Home
 *
 * @param screen point to the screen handle
 */
	selLCD.SendCmd(lcd, 0x02);
}

static int lcdl_Home(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);

	selLCD.Home(lcd);

	return 0;
}

static void lcdc_SetDDRAM(struct LCDscreen *lcd, uint8_t pos){
/** 
 * @brief Set display ram pointer
 *
 * @function SetDDRAM
 *
 * @param screen point to the screen handle
 * @tparam uint8_t position (<80 otherwise reset to 0)
 */
	if(pos > 0xe8)
		pos = 0;

	selLCD.SendCmd(lcd, 0x80 | pos);
}

static int lcdl_SetDDRAM(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	uint8_t pos = lua_toboolean(L, 2);

	selLCD.SetDDRAM(lcd, pos);

	return 0;
}

static void lcdc_SetCursor(struct LCDscreen *lcd, uint8_t x, uint8_t y){
/** 
 * @brief set the cursor at x,y position 
 *
 * @function SetCursor
 *
 * @param screen point to the screen handle
 * @param uint8_t x position
 * @param uint8_t y position
 *
 * Notez-bien : there is no boundary check. Up to the developer to know what
 *	it is doing.
 */
 	uint8_t p;

	switch(y){
	case 1:
		p = 0x40; break;
	case 2:
		p = 0x14; break;
	case 3:
		p = 0x54; break;
	default:
		p = 0x00;
	}
	p += x;
	selLCD.SetDDRAM(lcd, p);
}

static int lcdl_SetCursor(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	uint8_t x = lua_tonumber(L, 2);
	uint8_t y = lua_tonumber(L, 3);

	selLCD.SetCursor(lcd, x,y);

	return 0;
}

static void lcdc_SetCGRAM(struct LCDscreen *lcd, uint8_t pos){
/** 
 * @brief Set custom char ram pointer
 *
 * @function SetCGRAM
 *
 * @param screen point to the screen handle
 * @tparam uint8_t adresse 
 */
	if(pos > 7)
		pos = 0;

	selLCD.SendCmd(lcd, 0x40 | pos << 3);
}

static int lcdl_SetChar(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	uint8_t nchar = lua_tonumber(L, 2);

	if(!lua_istable(L, 3))
		luaL_error(L, "SetChar() 3rd argument is expected to be an array of strings");

	selLCD.SetCGRAM(lcd, nchar);

	for(size_t i=0; i<lua_rawlen(L,3); i++){
		lua_rawgeti(L, 3, i+1);
		const char *pat = luaL_checkstring(L, -1);

		uint8_t v=0;
		for(;*pat;pat++){
			v <<=1;
			if(*pat!=' ' && *pat!='0')
				v |= 1;
		}
		lua_pop(L,1);

		selLCD.SendData(lcd, v);
	}



	return 0;
}

static void lcdc_WriteString(struct LCDscreen *lcd, const char *txt){
/** 
 * @brief Write a characters string to the screen.
 *
 * @function WriteString
 *
 * @param screen point to the screen handle
 * @param string to be displayed
 *
 * Notez-bien : there is no limits, up to the programmer to know
 * what it's doing.
 */
	for(;*txt; txt++)
		selLCD.SendData(lcd, *txt);
}

static int lcdl_WriteString(lua_State *L){
	struct LCDscreen *lcd = checkSelLCD(L);
	const char *s = luaL_checkstring(L, 2);

	selLCD.WriteString(lcd, s);

	return 0;
}

static const struct luaL_Reg LCDM[] = {
	{"Shutdown", lcdl_Shutdown},
	{"Backlight", lcdl_Backlight},
	{"DisplayCtl", lcdl_DisplayCtl},
	{"EntryCtl", lcdl_EntryCtl},
	{"Clear", lcdl_Clear},
	{"Home", lcdl_Home},
	{"SetDDRAM", lcdl_SetDDRAM},
	{"SetCursor", lcdl_SetCursor},
	{"WriteString", lcdl_WriteString},
	{"SetChar", lcdl_SetChar},
	{NULL, NULL}    /* End of definition */
};

static const struct luaL_Reg LCDLib[] = {
	{"Init", lcdl_Init},
	{"Attach", lcdl_Init},
	{"SetTiming", lcdl_SetTiming},
	{NULL, NULL}    /* End of definition */
};

static void registerSelLCD(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelLCD", LCDLib);
	selLua->objFuncs(L, "SelLCD", LCDM);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Other mandatory modules */
	selLua =  (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selLCD, "SelLCD", SELLCD_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selLCD);

	if(selLua){	/* Only if Lua is used */
		registerSelLCD(NULL);
		selLua->AddStartupFunc(registerSelLCD);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

		/* Default timings */

	selLCD.clock_pulse = 500;
	selLCD.clock_process = 4100;

		/* This low level can be overwritten for example to use 1-Wire instead
		 * of I2C
		 */
	selLCD.SendQuarter = lcdc_SendQuarter;

		/* Callbacks */

	selLCD.Init = lcdc_Init;
	selLCD.Shutdown = lcdc_Shutdown;
	selLCD.SendCmd = lcdc_SendCmd;
	selLCD.SendData = lcdc_SendData;
	selLCD.Backlight = lcdc_Backlight;
	selLCD.DisplayCtl = lcdc_DisplayCtl;
	selLCD.EntryCtl = lcdc_EntryCtl;
	selLCD.Clear = lcdc_Clear;
	selLCD.Home = lcdc_Home;
	selLCD.SetDDRAM = lcdc_SetDDRAM;
	selLCD.SetCGRAM = lcdc_SetCGRAM;
	selLCD.SetCursor = lcdc_SetCursor;
	selLCD.WriteString = lcdc_WriteString;

	return true;
}
