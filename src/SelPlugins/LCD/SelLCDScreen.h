/***
 * LCD screen impersonation
 */

#ifndef SELLCDSCREEN_H
#define SELLCDSCREEN_H

#include "SelLCDSurface.h"

struct SelLCDScreen {
	struct SelLCDSurface primary;	// Screen own physical surface

	int bus;		/* I2C bus file descriptor */
	bool backlight;	/* is backlight enabled */

	useconds_t clock_pulse;		/* 'E' clock */
	useconds_t clock_process;	/* time to process */
};

#endif
