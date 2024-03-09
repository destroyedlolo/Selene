/* SelTimer.h
 *
 * Multi purposes and versatile timer
 *
 * Have a look and respect Selene Licence.
 */
#ifndef SELTIMER_VERSION

#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELTIMER_VERSION 1

#ifdef __cplusplus
extern "C"
{
#endif

struct SelTimer {
	struct SelModule module;

		/* Call backs */
};

#ifdef __cplusplus
}
#endif

#endif
