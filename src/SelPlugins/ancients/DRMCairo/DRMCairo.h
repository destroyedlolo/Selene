/* DRMCairo.h
 *
 * Framebuffer graphics using DRM/KMS and Cairo.
 *
 * Have a look and respect Selene Licence.
 */

#ifndef DRMCAIRO_H
#define DRMCAIRO_H

#include <Selene/SelPlug-in/SelDRMCairo.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

extern struct SelDRMCairo dc_selDRMCairo;

#include <pthread.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#ifndef KMS_MISSING
#	include <libkms.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H

	/* Global stuffs for this plugin.
	 * As global, no need to create an object : everything is handled through
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
	bool drm;	/* true if DRM, false if FB */
	int fd;	/* Fd corresponding to the card */

	drmModeResPtr resources;
	drmModeConnectorPtr connector;
	drmModeEncoderPtr encoder;
	drmModeCrtcPtr orig_crtc;	/* original CRTC backup */
#ifndef KMS_MISSING
	struct kms_driver *kms;
	struct kms_bo *bo;
#endif
	uint32_t pitch;
	uint32_t handle;
	lua_Number w,h;
	size_t screensize;
	void *map_buf;
	uint32_t fb;

	struct SelDCSurface primary_surface;	/* Cairo's primary surface */

/*	uint32_t offsets[4]; */
};

extern void _include_SelDCCard(lua_State *);
extern void _include_SelDCSurface(lua_State *);
extern void _include_SelDCSurfaceImage(lua_State *);
extern void _include_SelDCFont(lua_State *);
extern void _include_SelDCPattern(lua_State *);
#endif
