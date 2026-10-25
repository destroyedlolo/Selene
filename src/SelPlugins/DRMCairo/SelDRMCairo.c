/***
 * Framebuffer graphics using DRM/KMS and Cairo.
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X or Wayland.
 *

@classmod SelDRMCairo

 * 25/10/2026 LF : version 2
 */

#include <Selene/SelPlug-in/SelDRMCairo/SelDRMCairo.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

struct SelDRMCairo sdrmc_selDRMCairo;

struct SeleneCore *sdrmc_selCore;
struct SelLog *sdrmc_selLog;
struct SelLua *sdrmc_selLua;

static bool sdrmc_findScreen(const char *device, const char *name, struct selDRMScreen *result){
	result->device_fd = open(device, O_RDWR | O_CLOEXEC);
	if(result->device_fd < 0){
		sdrmc_selLog->Log('E', "%s : %s", device, strerror(errno));
		return false;
	}

	drmModeRes *res = drmModeGetResources(result->device_fd);
	if(!res){
		sdrmc_selLog->Log('E', "Can't get DRM resources : %s", strerror(errno));
		close(result->device_fd);
		return false;
	}

	bool found = false;

		/* Loops against connector */
	for(int i = 0; i < res->count_connectors && !found; ++i){
		drmModeConnector *conn = drmModeGetConnector(result->device_fd, res->connectors[i]);
		if(!conn)
			continue;

		if(conn->connection != DRM_MODE_CONNECTED){
			drmModeFreeConnector(conn);
			continue;	
		}

			/* Connector name */
        const char *type_name = "Unknown";
		if(conn->connector_type == DRM_MODE_CONNECTOR_HDMIA)
			type_name = "HDMI-A";
		else if(conn->connector_type == DRM_MODE_CONNECTOR_HDMIB)
			type_name = "HDMI-B";
		else if(conn->connector_type == DRM_MODE_CONNECTOR_DisplayPort)
			type_name = "DP";
		else if(conn->connector_type == DRM_MODE_CONNECTOR_eDP)
			type_name = "eDP";
		else if(conn->connector_type == DRM_MODE_CONNECTOR_VGA)
			type_name = "VGA";

		snprintf(result->name, sizeof(result->name), "%s-%u", type_name, conn->connector_type_id);

			/* The name has been provided */
		if(name != NULL && strcmp(result->name, name) != 0){
			drmModeFreeConnector(conn);
			continue;
        }

			/* Find out the CRTC */
		result->crtc_id = 0;
		if(conn->encoder_id){
			drmModeEncoder *enc = drmModeGetEncoder(result->device_fd, conn->encoder_id);
			if(enc){
				result->crtc_id = enc->crtc_id;
				drmModeFreeEncoder(enc);
			}
		}
		if(!result->crtc_id){
			for(int j = 0; j < conn->count_encoders; ++j){
				drmModeEncoder *enc = drmModeGetEncoder(result->device_fd, conn->encoders[j]);
				if(enc){
					for(int c = 0; c < res->count_crtcs; ++c){
						if(enc->possible_crtcs & (1 << c)){
							result->crtc_id = res->crtcs[c];
							break;
						}
					}
					drmModeFreeEncoder(enc);
					if(result->crtc_id)
						break;
				}
			}
		}

			/* check if active and with valid mode */
		if(result->crtc_id){
			drmModeCrtc *crtc = drmModeGetCrtc(result->device_fd, result->crtc_id);
			if(crtc && crtc->mode_valid){
				result->connector_id = conn->connector_id;
				result->width = crtc->mode.hdisplay;
				result->height = crtc->mode.vdisplay;

				found = true;
			}
			if(crtc)
				drmModeFreeCrtc(crtc);
		}

		drmModeFreeConnector(conn);
	}

	drmModeFreeResources(res);

	if(!found)
		close(result->device_fd);
	return found;
}

static void registerSelDRMCairo(lua_State *L){
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/

bool InitModule( void ){
		/* Core modules */
	sdrmc_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!sdrmc_selCore)
		return false;

	sdrmc_selLog = (struct SelLog *)sdrmc_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!sdrmc_selLog)
		return false;

		/* Other mandatory modules */
	sdrmc_selLua = (struct SelLua *)sdrmc_selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&sdrmc_selDRMCairo, "SelDRMCairo", SELDRMCAIRO_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&sdrmc_selDRMCairo);

	if(sdrmc_selLua){	/* Only if Lua is used */
		registerSelDRMCairo(NULL);
		sdrmc_selLua->AddStartupFunc(registerSelDRMCairo);
	}
#ifdef DEBUG
	else
		sdrmc_selLog->Log('D', "SelLua not loaded");
#endif

		/* Callbacks */
	sdrmc_selDRMCairo.findScreen = sdrmc_findScreen;

	return true;
}
