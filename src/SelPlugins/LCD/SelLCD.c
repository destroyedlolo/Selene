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
#include <assert.h>

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

	selLCD.Init(lcd, nbus, addr, twolines, y11);

	return 1;
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

static void lcdc_SetDDRAM(struct LCDscreen *lcd, uint8_t pos){
/** 
 * @brief Set display ram pointer
 *
 * @function SetDDRAM
 *
 * @param screen point to the screen handle
 * @tparam uint8_t position (<80 otherwise reset to 0)
 */
	if(pos > 79)
		pos = 0;

	selLCD.SendCmd(lcd, 0x80 | pos);
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
	selLCD.SetDDRAM(lcd, y*0x40 + x);
}

static const struct luaL_Reg LCDM[] = {
	{"Shutdown", lcdl_Shutdown},
	{"Backlight", lcdl_Backlight},

	{NULL, NULL}    /* End of definition */
};

static const struct luaL_Reg LCDLib[] = {
	{"Init", lcdl_Init},
	{"Attach", lcdl_Init},
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
	selLCD.SetCursor = lcdc_SetCursor;
	selLCD.WriteString = lcdc_WriteString;

	return true;
}
