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
#include <libkms/libkms.h>

#include "../../SeleneLibrary/libSelene.h"

struct DCCard {
	int fd;	/* Fd corresponding to the card */
	drmModeResPtr resources;
	drmModeConnectorPtr connector;
	drmModeEncoderPtr encoder;
	struct kms_driver *kms;
	struct kms_bo *bo;
	uint32_t handles[4];
	uint32_t pitches[4];
	uint32_t offsets[4];
};

extern void initDRMCairo(lua_State *);

extern void _include_SelDCCard(lua_State *);

#	endif
#endif
