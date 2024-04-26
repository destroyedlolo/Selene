/* SelOLED.h
 *
 * Graphical framework for small displays
 * like SSD1306 I2C driven OLED display.
 *
 *	Depends on https://github.com/destroyedlolo/ArduiPi_OLED
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELOLED_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELOLED_VERSION 2

struct SelOLED {
	struct SelModule module;

		/* Call backs */
};

#endif
