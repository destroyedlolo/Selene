/* SelEvent.h
 *
 * Linux event interface
 *
 * Have a look and respect Selene Licence.
 */
#ifndef SELEVENT_VERSION

#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELEVENT_VERSION 1

struct SelEvent {
	struct SelModule module;

		/* Call backs */
};

#endif
