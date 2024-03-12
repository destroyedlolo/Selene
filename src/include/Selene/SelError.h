/* SelError.h
 *
 * Error management.
 *
 * As there is no clean way to create exception in Lua, SelError is a way
 * to report errors from function having multiple returns (as WaitForr())
 *
 * Have a look and respect Selene Licence.
 */
#ifndef SELERROR_VERSION

#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELERROR_VERSION 1

#ifdef __cplusplus
extern "C"
{
#endif

struct SelError {
	struct SelModule module;

		/* Call backs */
};

#ifdef __cplusplus
}
#endif

#endif
