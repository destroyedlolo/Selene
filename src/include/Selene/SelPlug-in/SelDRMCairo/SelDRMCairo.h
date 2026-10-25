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

#ifdef __cplusplus
extern "C"
{
#endif

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

#if __has_include(<cairo.h>)
#	include <cairo.h>
#elif __has_include(<cairo/cairo.h>)
#	include <cairo/cairo.h>
#else
#	error "Cairo not found"
#endif

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELDRMCAIRO_VERSION 2

/* ****
 * DRM's objects description
 */

#define MAX_SELDRMSCN_NAME_LEN 32

struct selDRMScreen {
	int device_fd;

	uint32_t connector_id;
	uint32_t crtc_id;
	uint32_t width;
	uint32_t height;
	char name[MAX_SELDRMSCN_NAME_LEN];
};

struct SelDRMCairo {
	struct SelModule module;

		/* ***
		 * Callbacks
		 * ***/

		/* Find the active screen by its name
		 * -> device : device to open (/dev/dri/card0)
		 * -> name : name of the screen we're looking for (NULL for the 1st active one)
		 * <- false if not found (result is unchanged)
		 */
	bool (*findScreen)(const char *device, const char *name, struct selDRMScreen *result);
};

#ifdef __cplusplus
}
#endif

#endif
