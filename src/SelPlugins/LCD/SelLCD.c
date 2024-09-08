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

static void SendQuarter(struct LCDscreen *lcd, uint8_t b){
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
	SendQuarter(lcd, t);

	t &= 0x0f;	/* Keep only control bits */
	t |= dt << 4;	/* send less significant quarter */
	SendQuarter(lcd, t);
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
	SendQuarter(lcd, t);

	t &= 0x0f;	/* Keep only control bits */
	t |= dt << 4;	/* send less significant quarter */
	SendQuarter(lcd, t);
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
 * @param screen point to the screen handle
 * @tparam boolean increment the cursor when a character is sent
 * @tparam boolean shift the screen if the cursor leaves it
 */
	uint8_t t = 0x04;		/* Enty control */
	t |= inc ? 0x02 : 0x00;
	t |= shift ? 0x01 : 0x00;

	selLCD.SendCmd(lcd, t);
}

static const struct luaL_Reg LCDLib[] = {
	{NULL, NULL}    /* End of definition */
};

static void registerSelLCD(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelLCD", LCDLib);
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

	registerSelLCD(NULL);
	selLua->AddStartupFunc(registerSelLCD);

		/* Callbacks */
	selLCD.Init = lcdc_Init;
	selLCD.SendCmd = lcdc_SendCmd;
	selLCD.SendData = lcdc_SendData;
	selLCD.DisplayCtl = lcdc_DisplayCtl;
	selLCD.EntryCtl = lcdc_EntryCtl;

	return true;
}
