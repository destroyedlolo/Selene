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

#include "../../SeleneLibrary/libSelene.h"

struct SelDCSurface {
	cairo_surface_t *surface;
	cairo_t *cr;

	double w,h;	/* Keep surface size as it seems it's not possible to retrieve it*/
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

extern void _include_SelDCCard(lua_State *);
extern void _include_SelDCSurface(lua_State *);
#	endif
#endif
