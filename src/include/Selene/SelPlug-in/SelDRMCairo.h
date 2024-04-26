/* SelDRMCairo.h
 *
 * Framebuffer graphics using DRM/KMS and Cairo.
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X.
 *
 * Depends on industry standard Cairo library
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELDRMCAIRO_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELCAIRO_VERSION 1

struct SelDRMCairo {
	struct SelModule module;

		/* Call backs */
};

#endif
