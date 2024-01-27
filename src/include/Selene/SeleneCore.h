/* SeleneCore.h
 *
 * Selene's core and helpers
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELENECORE_VERSION
#include "libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELENECORE_VERSION (LIBSELENE_VERSION + 1)

#ifdef __cplusplus
extern "C"
{
#endif

struct SeleneCore {
	struct SelModule module;
};

#ifdef __cplusplus
}
#endif

#endif
