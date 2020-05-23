/* DRMCairo.h
 *
 * This file contains stuffs related to initialisation of DRMCairo plug-in.
 *
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X.
 *
 * 10/05/2020 LF : Creation
 */
#ifndef SDRMCAIRO_H
#define SDRMCAIRO_H

#	ifdef USE_DRMCAIRO

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libkms.h>
#include <cairo.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../../SeleneLibrary/libSelene.h"

	/* Global stuffs for this plugin.
	 * As global, no need to create an object : everything is handled thru
	 * this global variable
	 */
extern struct DRMCairoContext {
	FT_Library  FT;
	pthread_mutex_t FT_mutex;	/* protect FT_New_Face and FT_Done_Face calls
								 * see https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html
								 */
} DMCContext;

struct selDCFont {
	cairo_font_face_t *cairo;
	FT_Face ft;
};

struct SelDCSurface {
	cairo_surface_t *surface;
	cairo_t *cr;

	lua_Number w,h;	/* Keep surface size as it seems it's not possible to retrieve it*/

		/* As managing inheritances of Lua's metatable is a tedious task
		 * (especially to manage check*() functions), I managed in another way :
		 * - each and every derived classes are "SelDCSurface"
		 * - field 'type' exposes the real nature of the object
		 *
		 * Notez-bien : as the time of writing, it's not really needed for
		 * 	some child classes (like DCSURFACE_PRIMARY) but it's to prepare
		 */
	enum DCSURFACE_TYPE {
		DCSURFACE,	/* Mother class */
		DCSURFACE_PRIMARY,
		DCSURFACE_SUBSURFACE,
		DCSURFACE_IMAGE
	} type;
};

struct DCCard {
	int fd;	/* Fd corresponding to the card */
	drmModeResPtr resources;
	drmModeConnectorPtr connector;
	drmModeEncoderPtr encoder;
	struct kms_driver *kms;
	drmModeCrtcPtr orig_crtc;	/* original CRTC backup */
	struct kms_bo *bo;
	uint32_t pitch;
	uint32_t handle;
	void *map_buf;
	uint32_t fb;
	struct SelDCSurface primary_surface;	/* Cairo's primary surface */

/*	uint32_t offsets[4]; */
};

extern void initDRMCairo(lua_State *);

extern struct selDCFont *checkSelDCFont(lua_State *, int);
extern cairo_pattern_t *checkSelDCPattern(lua_State *, int);

extern void internal_release_surface(struct SelDCSurface *);

extern void _include_SelDCCard(lua_State *);
extern void _include_SelDCSurface(lua_State *);
extern void _include_SelDCSurfaceImage(lua_State *);
extern void _include_SelDCFont(lua_State *);
extern void _include_SelDCPattern(lua_State *);

#	endif
#endif
