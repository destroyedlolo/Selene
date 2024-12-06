/***
 * Display messages on an LCD textual screen (like 1602 one)
 *
 * Based on https://fr.wikipedia.org/wiki/HD44780 
 * and inspired by BitBank https://github.com/bitbank2/LCD1602

@classmod SelLCD

 * 06/09/2024 LF : First version
 */

#include <Selene/SelPlug-in/SelLCD/SelLCDScreen.h>
#include "SelLCDShared.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <assert.h>

#if LUA_VERSION_NUM == 501
#	define lua_rawlen lua_objlen
#endif

struct SelLCD slcd_selLCD;

struct SeleneCore *slcd_selCore;
struct SelLog *slcd_selLog;
struct SelLua *slcd_selLua;

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


	/* ***
	 * Low level technical function
	 * ***/

static void lcdc_SendQuarter(struct SelLCDScreen *lcd, uint8_t b){
/* Send the provided quarter */

	write(lcd->bus, &b, 1);	/* Present the data on the gpio */
	usleep(lcd->clock_pulse);

	b |= ENABLE;		/* Rise 'E' */
	write(lcd->bus, &b, 1);
	usleep(lcd->clock_pulse);

	b &= ~(ENABLE);		/* Lower 'E' */
	write(lcd->bus, &b, 1);
	usleep(lcd->clock_process);
}

static void lcdc_SendCmd(struct SelLCDScreen *lcd, uint8_t dt){
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
	slcd_selLCD.SendQuarter(lcd, t);

	t &= 0x0f;	/* Keep only control bits */
	t |= dt << 4;	/* send less significant quarter */
	slcd_selLCD.SendQuarter(lcd, t);
}

static void lcdc_SendData(struct SelLCDScreen *lcd, uint8_t dt){
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
	slcd_selLCD.SendQuarter(lcd, t);

	t &= 0x0f;	/* Keep only control bits */
	t |= dt << 4;	/* send less significant quarter */
	slcd_selLCD.SendQuarter(lcd, t);
}


	/* ***
	 * Initialisation
	 * ***/

static bool lcdc_Init(struct SelLCDScreen *lcd, uint16_t bus_number, uint8_t address, bool multilines, bool y11){
/** 
 * @brief Initialize connection to the screen
 *
 * @function Init
 * @param screen point to the screen handle
 * @tparam uint16_t I2C bus number
 * @tparam uint8_t Screen I2C address
 * @tparam boolean true if the screen has more than 1 lines 
 * @tparam boolean true if the screen is 11 pixel hight
 * @treturn boolean false if we faced a technical error
 *
 * @warning : screen contrast (at least on my 2004) is different in multiline mode
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

		/* Default timings */
	lcd->clock_pulse = 500;
	lcd->clock_process = 4100;

	initExportedSurface((struct SelLCDSurface *)lcd,
		NULL,	/* No parent, we're primary */
		0,0,	/* let's guess the size */
		0,0,	/* no margin */
		lcd		/* We are the primary */
	);

		/* Initializing 
		 * SET + 4 bits mode
		 * We're sending the upper quarter first so 0x02 is really 0x20.
		 */
	slcd_selLCD.SendCmd(lcd, 0x02);

		/* Now sending the full configuration */
	slcd_selLCD.SendCmd(lcd, 0x20 | (multilines ? 0x08 : 0) | (y11 ? 0x04 : 0));
	
	return true;
}

static int lcdl_Init(lua_State *L){
	uint16_t nbus = luaL_checkinteger(L, 1);
	uint8_t addr = luaL_checkinteger(L, 2);
	bool multilines = lua_toboolean(L, 3);
	bool y11 = lua_toboolean(L, 4);

	struct SelLCDScreen *lcd = (struct SelLCDScreen *)lua_newuserdata(L, sizeof(struct SelLCDScreen));
	assert(lcd);
	slcd_selCore->initGenericSurface((struct SelModule *)&slcd_selLCD, (struct SelGenericSurface *)lcd);

	if(!slcd_selLCD.Init(lcd, nbus, addr, multilines, y11)){
		lua_pop(L, 1);
		return 0;
	}

	luaL_getmetatable(L, "SelLCD");
	lua_setmetatable(L, -2);

	return 1;
}


	/* ***
	 * APIs
	 * ***/

static struct SelLCDScreen *checkSelLCD(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCD");
	luaL_argcheck(L, r != NULL, 1, "'SelLCD' expected");

	return (struct SelLCDScreen *)r;
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
	struct SelLCDScreen *lcd = checkSelLCD(L);
	lcd->clock_pulse = luaL_checkinteger(L, 2);
	lcd->clock_process = luaL_checkinteger(L, 3);

	return 0;
}

static void lcdc_Shutdown(struct SelLCDScreen *lcd){
/**
 * @brief Turn off the screen
 *
 * @function Shutdown
 * @param screen point to the screen handle
 */
	slcd_selLCD.DisplayCtl(lcd, false, false, false);
	close(lcd->bus);
	lcd->bus = -1;
}

static int lcdl_Shutdown(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);

	slcd_selLCD.Shutdown(lcd);

	return 0;
}

static void lcdc_Backlight(struct SelLCDScreen *lcd, bool bl){
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
	struct SelLCDScreen *lcd = checkSelLCD(L);
	bool bl = lua_toboolean(L, 2);

	slcd_selLCD.Backlight(lcd, bl);

	return 0;
}

static void lcdc_DisplayCtl(struct SelLCDScreen *lcd, bool screen, bool cursor, bool blink){
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

	slcd_selLCD.SendCmd(lcd, t);
}

static int lcdl_DisplayCtl(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	bool screen = lua_toboolean(L, 2);
	bool cursor = lua_toboolean(L, 3);
	bool blink = lua_toboolean(L, 4);

	slcd_selLCD.DisplayCtl(lcd, screen, cursor, blink);

	return 0;
}

static void lcdc_EntryCtl(struct SelLCDScreen *lcd, bool inc, bool shift){
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

	slcd_selLCD.SendCmd(lcd, t);
}

static int lcdl_EntryCtl(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	bool inc = lua_toboolean(L, 2);
	bool shift = lua_toboolean(L, 3);

	slcd_selLCD.EntryCtl(lcd, inc, shift);

	return 0;
}

static void lcdc_Clear(struct SelLCDScreen *lcd){
/** 
 * @brief Clear the screen
 *
 * @function Clear
 *
 * @param screen point to the screen handle
 */
	slcd_selLCD.SendCmd(lcd, 0x01);
}

static int lcdl_Clear(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);

	slcd_selLCD.Clear(lcd);

	return 0;
}

static bool lcdc_Home(struct SelLCDScreen *lcd){
/** 
 * @brief Places cursor at up-left position
 *
 * @function Home
 *
 * @param screen point to the screen handle
 */
	slcd_selLCD.SendCmd(lcd, 0x02);

	return true;
}

static int lcdl_Home(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);

	slcd_selLCD.Home(lcd);

	return 0;
}

static void lcdc_SetDDRAM(struct SelLCDScreen *lcd, uint8_t pos){
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

	slcd_selLCD.SendCmd(lcd, 0x80 | pos);
}

static int lcdl_SetDDRAM(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	uint8_t pos = lua_toboolean(L, 2);

	slcd_selLCD.SetDDRAM(lcd, pos);

	return 0;
}

static bool lcdc_SetCursor(struct SelLCDScreen *lcd, uint16_t x, uint16_t y){
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
	slcd_selLCD.SetDDRAM(lcd, p);

	return true;
}

static int lcdl_SetCursor(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	uint16_t x = lua_tonumber(L, 2);
	uint16_t y = lua_tonumber(L, 3);

	slcd_selLCD.SetCursor(lcd, x,y);

	return 0;
}

static void lcdc_SetCGRAM(struct SelLCDScreen *lcd, uint8_t pos){
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

	slcd_selLCD.SendCmd(lcd, 0x40 | pos << 3);
}

static int lcdl_SetChar(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	uint8_t nchar = lua_tonumber(L, 2);

	if(!lua_istable(L, 3))
		luaL_error(L, "SetChar() 3rd argument is expected to be an array of strings");

	slcd_selLCD.SetCGRAM(lcd, nchar);

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

		slcd_selLCD.SendData(lcd, v);
	}

	return 0;
}

static void lcdc_WriteString(struct SelLCDScreen *lcd, const char *txt){
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
		slcd_selLCD.SendData(lcd, *txt);
}

static int lcdl_WriteString(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	const char *s = luaL_checkstring(L, 2);

	slcd_selLCD.WriteString(lcd, s);

	return 0;
}

/* There is strictly no way to detect the geometry of the screen.
 * So we are setting it manually.
 */

static void lcdc_SetSize(struct SelLCDScreen *lcd, uint8_t w, uint8_t h){
	lcd->primary.w = w;
	lcd->primary.h = h;
}

static bool lcdc_GetSize(struct SelLCDScreen *lcd, uint8_t *w, uint8_t *h){
	if(w)
		*w = lcd->primary.w;
	if(h)
		*h = lcd->primary.h;

	return true;
}

static int lcdl_SetSize(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	uint8_t w = lua_tonumber(L, 2);
	uint8_t h = lua_tonumber(L, 3);

	lcdc_SetSize(lcd, w,h);

	return 0;
}

static int lcdl_GetSize(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	uint8_t w,h;

	lcdc_GetSize(lcd, &w,&h);

	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	return 2;
}
	
static int lcdl_subSurface(lua_State *L){
	struct SelLCDScreen *lcd = checkSelLCD(L);
	uint8_t x = lua_tonumber(L, 2);
	uint8_t y = lua_tonumber(L, 3);
	uint8_t w = lua_tonumber(L, 4);
	uint8_t h = lua_tonumber(L, 5);

	struct SelLCDSurface *srf = (struct SelLCDSurface *)lcd->primary.obj.cb->subSurface(L, (struct SelGenericSurface *)lcd, x,y, w,h, lcd);
	if(!srf)
		return 0;

	luaL_getmetatable(L, "SelLCDSurface");
	lua_setmetatable(L, -2);

	return 1;
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
	{"SetSize", lcdl_SetSize},
	{"GetSize", lcdl_GetSize},
	{"SetTiming", lcdl_SetTiming},
	{"SubSurface", lcdl_subSurface},
	{NULL, NULL}    /* End of definition */
};

static const struct luaL_Reg LCDLib[] = {
	{"Init", lcdl_Init},
	{"Attach", lcdl_Init},
	{NULL, NULL}    /* End of definition */
};

static void registerSelLCD(lua_State *L){
	slcd_selLua->libCreateOrAddFuncs(L, "SelLCD", LCDLib);
	slcd_selLua->objFuncs(L, "SelLCD", LCDM);
	
	slcd_selLua->objFuncs(L, "SelLCDSurface", LCDSM);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/

bool InitModule( void ){
		/* Core modules */
	slcd_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!slcd_selCore)
		return false;

	slcd_selLog = (struct SelLog *)slcd_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!slcd_selLog)
		return false;

		/* Other mandatory modules */
	slcd_selLua =  (struct SelLua *)slcd_selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&slcd_selLCD, "SelLCD", SELLCD_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&slcd_selLCD);

	if(slcd_selLua){	/* Only if Lua is used */
		registerSelLCD(NULL);
		slcd_selLua->AddStartupFunc(registerSelLCD);
	}
#ifdef DEBUG
	else
		slcd_selLog->Log('D', "SelLua not loaded");
#endif

		/* This low level can be overwritten for example to use 1-Wire instead
		 * of I2C
		 */
	slcd_selLCD.SendQuarter = lcdc_SendQuarter;

		/* Callbacks */

	slcd_selLCD.Init = lcdc_Init;
	slcd_selLCD.Shutdown = lcdc_Shutdown;
	slcd_selLCD.SendCmd = lcdc_SendCmd;
	slcd_selLCD.SendData = lcdc_SendData;
	slcd_selLCD.SetSize = lcdc_SetSize;
	slcd_selLCD.GetSize = lcdc_GetSize;
	slcd_selLCD.Backlight = lcdc_Backlight;
	slcd_selLCD.DisplayCtl = lcdc_DisplayCtl;
	slcd_selLCD.EntryCtl = lcdc_EntryCtl;
	slcd_selLCD.Clear = lcdc_Clear;
	slcd_selLCD.Home = lcdc_Home;
	slcd_selLCD.SetDDRAM = lcdc_SetDDRAM;
	slcd_selLCD.SetCGRAM = lcdc_SetCGRAM;
	slcd_selLCD.SetCursor = lcdc_SetCursor;
	slcd_selLCD.WriteString = lcdc_WriteString;

	initSLSCallBacks();
	return true;
}
