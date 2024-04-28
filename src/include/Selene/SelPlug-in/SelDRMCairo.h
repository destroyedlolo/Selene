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

#include <cairo.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELCAIRO_VERSION 1

struct SelDCSurface;
struct selDCFont;

struct SelDRMCairo {
	struct SelModule module;

		/* Call backs */
		/* Avoid to export symbol as much as possible. 
		 * This preventing naming clash with other modules
		 * (like if using namespace)
		 */
	struct SeleneCore *selCore;
	struct SelLog *selLog;
	struct SelLua *selLua;

	void (*internal_release_surface)(struct SelDCSurface *);
	cairo_pattern_t *(*checkSelDCPattern)(lua_State *, int);
	struct selDCFont *(*checkSelDCFont)(lua_State *, int);
};

#endif
