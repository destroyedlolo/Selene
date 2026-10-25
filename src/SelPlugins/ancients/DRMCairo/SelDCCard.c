/***
 *
 * Direct Rendering Manager card management.
 *

@classmod SelDCCard

 *
 * 13/05/2020 LF : Creation
 * 18/10/2020 LF : Check if KMS is available (to compile under debian)
 * 	if not Framebuffer is mandatory
 * 28/09/2020 LF : Y size can be forced in OpenFB()
 * 24/04/2020 LF ! Migrate to v7
 *
 * sources :
 * 	https://waynewolf.github.io/2012/09/05/libdrm-samples/
 * 	https://events.static.linuxfound.org/sites/events/files/lcjpcojp13_pinchart.pdf
 * 	https://github.com/dvdhrm/docs/blob/master/drm-howto/modeset-vsync.c
 *
 * TODO : for the moment, it deals ONLY with 
 * 	- the 1st available connector (which is the native resolution)
 * 	- the 1st available mode
 */

#include "DRMCairo.h"

#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef DRMC_WITH_FB
#	include <sys/ioctl.h>
#	include <linux/fb.h>
#	include <sys/mman.h>
#endif

	/* Build test drawing funcs */
#define TEST

struct DCCard *checkSelDCCard(lua_State *L){
	struct DCCard **r = luaL_checkudata(L, 1, "SelDCCard");
	luaL_argcheck(L, r != NULL, 1, "'SelDCCard' expected");
	return *r;
}

#ifdef TEST
#include <math.h> /* M_PI */

static int TestDraw(lua_State *L){
	struct DCCard *card = checkSelDCCard(L);

	int i, j;

	/* Well, I'm lazy to transform this piece of code to Framebuffer especially
	 * because it's only for testing purpose.
	 * So raising an error
	 */
	if(!card->drm){
		dc_selDRMCairo.selLog->Log('E',"TestDraw() not implemented for Framebuffer");
		lua_pushnil(L);
		lua_pushstring(L, "TestDraw() not implemented for Framebuffer");
		return 2;
	}

	/* paint the buffer with colored tiles */
	for (j = 0; j < card->connector->modes[0].vdisplay; j++) {
		uint32_t *fb_ptr = (uint32_t*)((char*)card->map_buf + j * card->pitch);
		for (i = 0; i < card->connector->modes[0].hdisplay; i++) {
			div_t d = div(i, card->connector->modes[0].hdisplay);
			fb_ptr[i] =
				0x00130502 * (d.quot >> 6) +
				0x000a1120 * (d.rem >> 6);
		}
	}

	return 0;
}

static int TestDrawCairo(lua_State *L){
	struct DCCard *card = checkSelDCCard(L);
	cairo_t *cr = card->primary_surface.cr;

	/* Well, I'm lazy to transform this piece of code to Framebuffer especially
	 * because it's only for testing purpose.
	 * So raising an error
	 */
	if(!card->drm){
		dc_selDRMCairo.selLog->Log('E',"TestDraw() not implemented for Framebuffer");
		lua_pushnil(L);
		lua_pushstring(L, "TestDraw() not implemented for Framebuffer");
		return 2;
	}

	/* Use normalized coordinates hereinafter */
	cairo_scale (cr, card->connector->modes[0].hdisplay, card->connector->modes[0].vdisplay);

	/* rectangle stroke */
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_set_line_width (cr, 0.05);
	cairo_rectangle (cr, 0.1, 0.1, 0.3, 0.4);
	cairo_stroke (cr);

	/* circle fill */
	cairo_set_source_rgba(cr, 1, 0, 0, 0.5);
	cairo_arc(cr, 0.7, 0.3, 0.2, 0, 2 * M_PI);
	cairo_fill(cr);

	/* text */
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	cairo_select_font_face (cr, "Georgia",
    	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 0.1);
	cairo_move_to (cr, 0.1, 0.8);
	cairo_show_text (cr, "drawn with cairo");
	
	return 0;
}
#endif

static int GetPrimarySurface(lua_State *L){
	/*** Returns the surface corresponding to the physical display.
	 *
	 * **CAUTION :**  Drawing to this surface is directly reflected on the screen
	 *
	 * @function GetPrimarySurface
	 * @treturn SelDCSurface surface
	 *
	 */
	struct DCCard *card = checkSelDCCard(L);
	struct SelDCSurface *srf = (struct SelDCSurface *)lua_newuserdata(L, sizeof(struct SelDCSurface));
	assert(srf);
	luaL_getmetatable(L, "SelDCSurface");
	lua_setmetatable(L, -2);

	srf->surface = card->primary_surface.surface;
	cairo_surface_reference(card->primary_surface.surface);
	srf->cr = card->primary_surface.cr;
	cairo_reference(card->primary_surface.cr);

	srf->w = card->w;
	srf->h = card->h;
	srf->type = DCSURFACE_PRIMARY;

	return 1;
}

static int CountAvailableModes(lua_State *L){
	/** Return the number of mode available
	 *
	 * @function CountAvailableModes
	 * @treturn integer number of available modes
	 */
	struct DCCard *card = checkSelDCCard(L);

	if(!card){
		dc_selDRMCairo.selLog->Log('E',"CountAvailableModes() on a dead object");
		lua_pushnil(L);
		lua_pushstring(L, "CountAvailableModes() on a dead object");
		return 2;
	}

		/* Framebuffer is only a fallback, so we are cheating to only
		 * 1 mode : the active one
		 */
	lua_pushinteger(L, card->drm ? card->connector->count_modes : 0);
	return 1;
}

static int GetSize(lua_State *L){
	/** Return the size of the current connector
	 * as well as refresh frequency.
	 *
	 * @function GetSize
	 * @tparam integer mode_index If omitted, return the active one
	 * @treturn integer width
	 * @treturn integer height
	 * @treturn integer frequency
	 */
	struct DCCard *card = checkSelDCCard(L);
	lua_Number idx = lua_tonumber(L, 2);

	if(!card){
		dc_selDRMCairo.selLog->Log('E',"GetSize() on a dead object");
		lua_pushnil(L);
		lua_pushstring(L, "GetSize() on a dead object");
		return 2;
	}

	/*
	 * No need to check anything as the object hasn't been created
	 * in case of error
	 */

	lua_pushinteger(L, card->drm ? card->connector->modes[(int)idx].hdisplay : card->w);
	lua_pushinteger(L, card->drm ? card->connector->modes[(int)idx].vdisplay : card->h);
	lua_pushinteger(L, card->drm ? card->connector->modes[(int)idx].vrefresh : 0 );
	return 3;
}

static void clean_card(struct DCCard *ctx){
	/* Clean card context structure
	 *
	 *	fd is not checked as if it failed to be opened, it's before this
	 *	structure allocation.
	 */

	if(ctx->primary_surface.cr)
		cairo_destroy(ctx->primary_surface.cr);
	if(ctx->primary_surface.surface)
		cairo_surface_destroy(ctx->primary_surface.surface);
	if(ctx->drm == true){
		if(ctx->fb)
			drmModeRmFB(ctx->fd, ctx->fb);
#ifndef KMS_MISSING
		if(ctx->map_buf)
			kms_bo_unmap(ctx->bo);
		if(ctx->bo)
			kms_bo_destroy(&(ctx->bo));
		if(ctx->kms)
			kms_destroy(&(ctx->kms));
#endif
		if(ctx->encoder)
			drmModeFreeEncoder(ctx->encoder);
		if(ctx->connector)
			drmModeFreeConnector(ctx->connector);
		if(ctx->resources)
			 drmModeFreeResources(ctx->resources);
#ifdef DRMC_WITH_FB
	} else {	/* Framebuffer */
		if(ctx->map_buf != (void *)-1)
			munmap(ctx->map_buf, ctx->screensize);
#endif
	}

	close(ctx->fd);
	free(ctx);
}

static int ReleaseCard(lua_State *L){
	/** Release a card and free all resources
	 *
	 * @function Release
	 */

	struct DCCard *card = checkSelDCCard(L);
	clean_card(card);

	return 0;
}

static int Open(lua_State *L){
	/** Initialise a card
	 *
	 * @function Open
	 * @tparam string card path (if not set /dev/dri/card0)
	 * @raise Fails if KMS is missing (Debian :( :( )
	 * @treturn SelDCCard card
	 */

#ifdef KMS_MISSING
	dc_selDRMCairo.selLog->Log('E',"No KMS");
	lua_pushnil(L);
	lua_pushstring(L, "No KMS");
	return 2;

#else

	const char *card = "/dev/dri/card0";
	uint64_t has_dumb;
	cairo_status_t err;


		/***
		 * Create Lua returned object
		 ***/
	struct DCCard **q = lua_newuserdata(L, sizeof(struct DCCard *));
	assert(q);
	assert( (*q = malloc(sizeof(struct DCCard))) );
	memset( *q, 0, sizeof(struct DCCard) );

	luaL_getmetatable(L, "SelDCCard");
	lua_setmetatable(L, -2);


		/****
		 * Specified card ?
		 ****/
	if( lua_isstring(L,1) )
		card = lua_tostring(L,1);

		/****
		 * 	Open the card device
		 ****/
	if(((*q)->fd = open(card, O_RDWR|O_CLOEXEC)) < 0){
		free(*q);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"%s : %s", card, strerror(errno));
		return 2;
	}
	(*q)->drm = true;


		/***
		 * Verify dumb buffers is supported 
		 ***/
	if(drmGetCap((*q)->fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb){
		close((*q)->fd);
		free(*q);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "dumb buffers is not supported");
		dc_selDRMCairo.selLog->Log('E',"%s : dumb buffers is not supported", card);
		return 2;
	}


		/****
		 *	Get display device
		 ****/
	if(!((*q)->resources = drmModeGetResources((*q)->fd))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "drmModeGetResources() failed");
		dc_selDRMCairo.selLog->Log('E',"%s : %s", card, strerror(errno));
		clean_card(t);
		return 2;
	}


		/*****
		 * Lookup for 1st connected connector 
		 *****/
	for(int i=0; i< (*q)->resources->count_connectors; i++){
		drmModeConnectorPtr c = drmModeGetConnector((*q)->fd, (*q)->resources->connectors[i]);

		if(c->connection == DRM_MODE_CONNECTED){
			(*q)->connector = c;
			break;
		} else
			drmModeFreeConnector(c);
	}
	if(!(*q)->connector){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "No connector found");
		dc_selDRMCairo.selLog->Log('E',"No connector found");
		clean_card(t);
		return 2;
	}

	if(!(*q)->connector->count_modes){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "No valid mode found");
		dc_selDRMCairo.selLog->Log('E',"No valid mode found");
		clean_card(t);
		return 2;
	}

	dc_selDRMCairo.selLog->Log('D',"Used resolution : %ix%i", (*q)->connector->modes[0].hdisplay, (*q)->connector->modes[0].vdisplay);

	/***
	 * Get encoder
	 ***/

	if((*q)->connector->encoder_id){
		(*q)->encoder = drmModeGetEncoder((*q)->fd, (*q)->connector->encoder_id);
	} else {
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "No encoder found");
		dc_selDRMCairo.selLog->Log('E',"No encoder found");
		clean_card(t);
		return 2;
	}

	if(!((*q)->orig_crtc = drmModeGetCrtc((*q)->fd, (*q)->encoder->crtc_id))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "Can't get old CRTC");
		dc_selDRMCairo.selLog->Log('E',"Can't get old CRTC");
		clean_card(t);
		return 2;
	}
	
	/***
	 * KMS driver creation
	 ***/

	if(kms_create((*q)->fd, &((*q)->kms))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"KMS creation : %s", strerror(errno));
		free(t);
		return 2;
	}


	/***
	 * Create the Buffer Object (bo)
	 ***/

	unsigned bo_attribs[] = {
		KMS_WIDTH,   (*q)->connector->modes[0].hdisplay, 
		KMS_HEIGHT,  (*q)->connector->modes[0].vdisplay,
		KMS_BO_TYPE, KMS_BO_TYPE_SCANOUT_X8R8G8B8,
		KMS_TERMINATE_PROP_LIST
	};
	if(kms_bo_create((*q)->kms, bo_attribs, &((*q)->bo))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"BO creation : %s", strerror(errno));
		free(t);
		return 2;
	}
	kms_bo_get_prop((*q)->bo, KMS_PITCH, &((*q)->pitch));
	kms_bo_get_prop((*q)->bo, KMS_HANDLE, &((*q)->handle));


	/***
	 * Map Buffer Object
	 ***/

	if(kms_bo_map((*q)->bo, &((*q)->map_buf))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"BO mapping : %s", strerror(errno));
		free(t);
		return 2;
	}


	/***
	 * Get the Frame buffer
	 ***/

	if(drmModeAddFB(
	  (*q)->fd, 
	  (*q)->connector->modes[0].hdisplay, 
	  (*q)->connector->modes[0].vdisplay, 
	  24, 32,	/* depth & bits per pixel */
	  (*q)->pitch, 
	  (*q)->handle, &((*q)->fb))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"BO mapping : %s", strerror(errno));
		free(t);
		return 2;
	}

	if(drmModeSetCrtc(
	  (*q)->fd, (*q)->encoder->crtc_id, (*q)->fb,
	  0,0, /* x,y */
	  &((*q)->connector->connector_id), 1,	/* number of connectors */
	  (*q)->connector->modes)){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"set display mode : %s", strerror(errno));
		free(t);
		return 2;		
	}


	/***
	 * Build Cairo's primary surface
	 ***/

	(*q)->primary_surface.surface = cairo_image_surface_create_for_data(
		(*q)->map_buf,
		CAIRO_FORMAT_ARGB32,
        (*q)->connector->modes[0].hdisplay, (*q)->connector->modes[0].vdisplay,
		(*q)->pitch);

	(*q)->primary_surface.cr = cairo_create((*q)->primary_surface.surface);
	if( (err=cairo_status((*q)->primary_surface.cr)) != CAIRO_STATUS_SUCCESS){
		struct DCCard *t = *q;
		cairo_destroy((*q)->primary_surface.cr);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L,cairo_status_to_string(err));
		lua_pushstring(L, "Unable to create Cairo's surface");
		dc_selDRMCairo.selLog->Log('E',"Unable to create Cairo's surface");
		free(t);
		return 3;
	}

	(*q)->w = (double)((*q)->connector->modes[0].hdisplay);
	(*q)->h = (double)((*q)->connector->modes[0].vdisplay);
	return 1;
#endif
}

#ifdef DRMC_WITH_FB

/* From :
 * https://gitlab.com/cairo/cairo-demos/blob/master/fbdev/cairo-fb.c
 */

static int OpenFB(lua_State *L){
	/** Initialise a card with framebuffer workaround.
	 *
	 * Useful if KMS is missing. 
	 * But need to be compiled with **DRMC\_WITH\_FB** set.
	 *
	 * @function OpenFB
	 * @tparam string card_path if not set, default to /dev/fb0
	 * @tparam integer vertical_size (optional, force vertical size, otherwise use its physical one
	 * @treturn SelDCCard card
	 *
	 * @usage
card,err,msg = SelDCCard.Open()
if not card then
    print("*E* DRM :", err,msg)
    if SELPLUG_DRMCairo_FBEXTENSION then
        local fb_fch = "/dev/fb1"
        if not file_exists(fb_fch) then
            fb_fch = "/dev/fb0"
        end
        card,err,msg = SelDCCard.OpenFB(fb_fch)
        if not card then
            print("*E* ".. fb_fch .." :", err,msg)
            os.exit()
        else
            print("*I* ok with Framebuffer")
        end
    else
        os.exit()
    end
end
	 */
	const char *card = "/dev/fb0";
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	cairo_status_t err;
	unsigned int forced_y = 0;
	

		/***
		 * Create Lua returned object
		 ***/
	struct DCCard **q = lua_newuserdata(L, sizeof(struct DCCard *));
	assert(q);
	assert( (*q = malloc(sizeof(struct DCCard))) );
	memset( *q, 0, sizeof(struct DCCard) );

	luaL_getmetatable(L, "SelDCCard");
	lua_setmetatable(L, -2);
	
		/****
		 * Specified card ?
		 ****/
	if( lua_isstring(L,1) )
		card = lua_tostring(L,1);

	if( lua_isnumber(L,2) )
		forced_y = lua_tointeger(L,2);

		/****
		 * 	Open the card device
		 ****/
	if(((*q)->fd = open(card, O_RDWR|O_CLOEXEC)) < 0){
		free(*q);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"%s : %s", card, strerror(errno));
		return 2;
	}
	(*q)->drm = false;

	if(ioctl((*q)->fd, FBIOGET_FSCREENINFO, &finfo)){
		close((*q)->fd);
		free(*q);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"FScreen info %s : %s", card, strerror(errno));
		return 2;
	}

	if(ioctl((*q)->fd, FBIOGET_VSCREENINFO, &vinfo)){
		close((*q)->fd);
		free(*q);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"VScreen info %s : %s", card, strerror(errno));
		return 2;
	}

	if(!forced_y)
		forced_y = vinfo.yres;

	(*q)->screensize = vinfo.xres_virtual * forced_y * vinfo.bits_per_pixel / 8;

	(*q)->map_buf = mmap(0, (*q)->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, (*q)->fd, 0);

	if((*q)->map_buf == (void *)-1){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		dc_selDRMCairo.selLog->Log('E',"%s : %s", card, strerror(errno));
		clean_card(t);
		return 2;
	}


	/***
	 * Build Cairo's primary surface
	 ***/

	(*q)->primary_surface.surface = cairo_image_surface_create_for_data(
		(*q)->map_buf,
		CAIRO_FORMAT_ARGB32,
		vinfo.xres_virtual, forced_y,
		finfo.line_length
	);
	(*q)->primary_surface.cr = cairo_create((*q)->primary_surface.surface);
	if( (err=cairo_status((*q)->primary_surface.cr)) != CAIRO_STATUS_SUCCESS){
		struct DCCard *t = *q;
		cairo_destroy((*q)->primary_surface.cr);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L,cairo_status_to_string(err));
		lua_pushstring(L, "Unable to create Cairo's surface");
		dc_selDRMCairo.selLog->Log('E',"Unable to create Cairo's surface");
		free(t);
		return 3;
	}
	(*q)->w = (double)(vinfo.xres_virtual);
	(*q)->h = (double)(forced_y);

	return 1;
}
#endif

	/* Object's own functions */
static const struct luaL_Reg SelDCCardM[] = {
#ifdef TEST
	{"TestDraw", TestDraw},
	{"TestDrawCairo", TestDrawCairo},
#endif
	{"GetPrimarySurface", GetPrimarySurface},
	{"GetSize", GetSize},
	{"CountAvailableModes", CountAvailableModes},
	{"Release", ReleaseCard},
	{NULL, NULL}    /* End of definition */
};

	/* Type's functions */
static const struct luaL_Reg SelDCCardLib[] = {
	{"Open", Open},
#ifdef DRMC_WITH_FB
	{"OpenFB", OpenFB},
#endif
	{NULL, NULL}    /* End of definition */
};

void _include_SelDCCard( lua_State *L ){
	dc_selDRMCairo.selLua->objFuncs( L, "SelDCCard", SelDCCardM );
	dc_selDRMCairo.selLua->libCreateOrAddFuncs( L, "SelDCCard", SelDCCardLib );
}

