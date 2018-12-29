/* SelOLED
 *
 * All shared definitions related to SSD1306 I2C driven OLED display
 *
 * 26/12/2018 LF : First version
 */

#ifndef SELOLED_H
#	define SELOLED_H

#	ifdef USE_OLED

#include "../../SeleneLibrary/libSelene.h"
#include <ArduiPi_OLED_C.h>

extern void initSelOLED(lua_State *);

#	endif
#endif
