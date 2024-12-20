/***
 * LCD screen impersonation
 */

#ifndef SELLCDSCREEN_H
#define SELLCDSCREEN_H

#include <Selene/SelPlug-in/SelLCD/SelLCDSurface.h>

struct SelLCDScreen {
	struct SelLCDSurface primary;	// Screen own physical surface

	int bus;		/* I2C bus file descriptor */
	bool backlight;	/* is backlight enabled */

	useconds_t clock_pulse;		/* 'E' clock */
	useconds_t clock_process;	/* time to process */
};

	/* struct SelLCDScreenLua is basically (and must be) a 
	 * "struct SelGenericSurfaceLua".
	 * It's goal is only to avoid zillion of cast.
	 */
struct SelLCDScreenLua {
	struct SelLCDScreen *storage;
};

#endif
