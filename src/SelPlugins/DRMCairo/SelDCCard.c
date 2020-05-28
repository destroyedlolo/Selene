/* SelDCCard.c
 *
 * This file contains stuffs related to DRM card managment.
 *
 * 13/05/2020 LF : Creation
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
#ifdef USE_DRMCAIRO

#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "DRMCairo.h"

	/* Build test drawing funcs */
#define TEST

struct DCCard **checkSelDCCard(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelDCCard");
	luaL_argcheck(L, r != NULL, 1, "'SelDCCard' expected");
	return (struct DCCard **)r;
}

#ifdef TEST
#include <math.h> /* M_PI */

static int TestDraw(lua_State *L){
	struct DCCard *card = *checkSelDCCard(L);

	int i, j;

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
	struct DCCard *card = *checkSelDCCard(L);
	cairo_t *cr = card->primary_surface.cr;

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

static int CountAvailableModes(lua_State *L){
	/* Return the number of mode available
	 * <- # of modes
	 */
	struct DCCard *card = *checkSelDCCard(L);

	if(!card){
		lua_pushnil(L);
		lua_pushstring(L, "CountAvailableModes() on a dead object");
		return 2;
	}

	lua_pushinteger(L, card->connector->count_modes);
	return 1;
}

static int GetPrimarySurface(lua_State *L){
	/* Get the surface corresponding to the physical display
	 * CAUTION : Drawing to this surface is directly reflected on the screen
	 */
	struct DCCard *card = *checkSelDCCard(L);
	struct SelDCSurface *srf = (struct SelDCSurface *)lua_newuserdata(L, sizeof(struct SelDCSurface));
	assert(srf);
	luaL_getmetatable(L, "SelDCSurface");
	lua_setmetatable(L, -2);

	srf->surface = card->primary_surface.surface;
	cairo_surface_reference(card->primary_surface.surface);
	srf->cr = card->primary_surface.cr;
	cairo_reference(card->primary_surface.cr);

	srf->w = (double)card->connector->modes[0].hdisplay;
	srf->h = (double)card->connector->modes[0].vdisplay;
	srf->type = DCSURFACE_PRIMARY;

	return 1;
}

static int GetSize(lua_State *L){
	/* Return the size of the current connector
	 * as well as refresh frequency
	 *
	 * No need to check anything as the object hasn't been created
	 * in case of error
	 *
	 * -> mode index. If omitted, return the active one
	 * <- width, height, frequency
	 */
	struct DCCard *card = *checkSelDCCard(L);
	lua_Number idx = lua_tonumber(L, 2);

	if(!card){
		lua_pushnil(L);
		lua_pushstring(L, "GetSize() on a dead object");
		return 2;
	}

	lua_pushinteger(L, card->connector->modes[(int)idx].hdisplay);
	lua_pushinteger(L, card->connector->modes[(int)idx].vdisplay);
	lua_pushinteger(L, card->connector->modes[(int)idx].vrefresh);
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
	if(ctx->fb)
		drmModeRmFB(ctx->fd, ctx->fb);
	if(ctx->map_buf)
		kms_bo_unmap(ctx->bo);
	if(ctx->bo)
		kms_bo_destroy(&(ctx->bo));
	if(ctx->kms)
		kms_destroy(&(ctx->kms));
	if(ctx->encoder)
		drmModeFreeEncoder(ctx->encoder);
	if(ctx->connector)
		drmModeFreeConnector(ctx->connector);
	if(ctx->resources)
		 drmModeFreeResources(ctx->resources);

	close(ctx->fd);
	free(ctx);
}

static int ReleaseCard(lua_State *L){
	/* Release a card and free all resources */

	struct DCCard *card = *checkSelDCCard(L);
	clean_card(card);

	return 0;
}

static int Open(lua_State *L){
	/* Initialise a card
	 * -> 1: card path (if not set /dev/dri/card0)
	 */
	const char *card = "/dev/dri/card0";
	uint64_t has_dumb;

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
#ifdef DEBUG
		printf("*E* %s : %s\n", card, strerror(errno));
#endif
		return 2;
	}


		/***
		 * Verify dumb buffers is supported 
		 ***/
	if(drmGetCap((*q)->fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb){
		close((*q)->fd);
		free(*q);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "dumb buffers is not supported");
#ifdef DEBUG
		printf("*E* %s : dumb buffers is not supported\n", card);
#endif
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
#ifdef DEBUG
		printf("*E* %s : %s\n", card, strerror(errno));
#endif
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
#ifdef DEBUG
		puts("*E* No connector found");
#endif
		clean_card(t);
		return 2;
	}

	if(!(*q)->connector->count_modes){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "No valid mode found");
#ifdef DEBUG
		puts("*E* No valid mode found");
#endif
		clean_card(t);
		return 2;
	}

#ifdef DEBUG
	printf("*I* Used resolution : %ix%i\n", (*q)->connector->modes[0].hdisplay, (*q)->connector->modes[0].vdisplay);
#endif


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
#ifdef DEBUG
		puts("*E* No encoder found");
#endif
		clean_card(t);
		return 2;
	}

	if(!((*q)->orig_crtc = drmModeGetCrtc((*q)->fd, (*q)->encoder->crtc_id))){
		struct DCCard *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "Can't get old CRTC");
#ifdef DEBUG
		puts("*E* Can't get old CRTC");
#endif
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
#ifdef DEBUG
		printf("*E* KMS creation : %s\n", strerror(errno));
#endif
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
#ifdef DEBUG
		printf("*E* BO creation : %s\n", strerror(errno));
#endif
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
#ifdef DEBUG
		printf("*E* BO mapping : %s\n", strerror(errno));
#endif
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
#ifdef DEBUG
		printf("*E* Getting FB : %s\n", strerror(errno));
#endif
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
#ifdef DEBUG
		printf("*E* set display mode : %s\n", strerror(errno));
#endif
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
	if(cairo_status((*q)->primary_surface.cr) != CAIRO_STATUS_SUCCESS){
		struct DCCard *t = *q;
		cairo_destroy((*q)->primary_surface.cr);
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "Unable to create Cairo's surface");
#ifdef DEBUG
		printf("*E* Unable to create Cairo's surface\n");
#endif
		free(t);
		return 2;
	}
	return 1;
}

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
	{NULL, NULL}    /* End of definition */
};


void _include_SelDCCard( lua_State *L ){
	libSel_objFuncs( L, "SelDCCard", SelDCCardM );
	libSel_libFuncs( L, "SelDCCard", SelDCCardLib );
}

#endif
