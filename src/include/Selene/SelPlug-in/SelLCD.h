/* SelLCD.h
 *
 * Display messages on an LCD textual screen (like 1602 one)
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLCD_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLCD_VERSION 2

struct SelLCD {
	struct SelModule module;

		/* Call backs */
};

#endif
