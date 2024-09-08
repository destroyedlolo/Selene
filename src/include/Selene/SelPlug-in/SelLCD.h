/* SelLCD.h
 *
 * Display messages on an LCD textual screen (like 1602 one)
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLCD_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

#include <unistd.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLCD_VERSION 2

struct LCDscreen {
	int bus;		/* I2C bus file descriptor */
	bool backlight;	/* is backlight enabled */
};

struct SelLCD {
	struct SelModule module;

	useconds_t clock_pulse;		/* 'E' clock */
	useconds_t clock_process;	/* time to process */
	
		/* Call backs */
	bool (*Init)(struct LCDscreen *, uint16_t bus_number, uint8_t address, bool twolines, bool y11);

	void (*SendCmd)(struct LCDscreen *, uint8_t);
	void (*SendData)(struct LCDscreen *, uint8_t);
};

#endif
