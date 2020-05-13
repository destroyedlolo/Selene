/* DRMCairo.c
 *
 * This file contains stuffs related to initialisation of DRMCairo plug-in.
 *
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X.
 *
 * 10/05/2020 LF : Creation
 *
 * TODO : for the moment, it deals ONLY with 
 * 	- the 1st available connector
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

static void clean_card(struct DRMCairoContext *ctx){
	/* Clean card context structure
	 *
	 *	fd is not checked as if it failed to be opened, it's before this
	 *	structure allocation.
	 */
	if(ctx->resources)
		 drmModeFreeResources(ctx->resources);
	if(ctx->connector)
		drmModeFreeConnector(ctx->connector);
	if(ctx->encoder)
		drmModeFreeEncoder(ctx->encoder);

	close(ctx->fd);
	free(ctx);
}

static int OpenGfxCard( lua_State *L ){
	/* Initialise a card
	 * -> 1: card path (if not set /dev/dri/card0)
	 */
	const char *card = "/dev/dri/card0";

		/***
		 * Create Lua returned object
		 ***/
	struct DRMCairoContext **q = lua_newuserdata(L, sizeof(struct DRMCairoContext *));
	assert(q);
	assert( (*q = malloc(sizeof(struct DRMCairoContext))) );
	memset( *q, 0, sizeof(struct DRMCairoContext) );

	luaL_getmetatable(L, "SelGfxCard");
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
		struct DRMCairoContext *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
#ifdef DEBUG
		printf("*E* %s : %s\n", card, strerror(errno));
#endif
		free(t);
		return 2;
	}


		/****
		 *	Get display device
		 ****/

	if(!((*q)->resources = drmModeGetResources((*q)->fd))){
		struct DRMCairoContext *t = *q;
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
		struct DRMCairoContext *t = *q;
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
		struct DRMCairoContext *t = *q;
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

	if((*q)->connector->encoder_id){
		(*q)->encoder = drmModeGetEncoder((*q)->fd, (*q)->connector->encoder_id);
	} else {
		struct DRMCairoContext *t = *q;
		lua_pop(L,1);		/* Remove return value */
		lua_pushnil(L);
		lua_pushstring(L, "No encoder found");
#ifdef DEBUG
		puts("*E* No encoder found");
#endif
		clean_card(t);
		return 2;
	}

	return 1;
}

static const struct luaL_Reg DRMCLib[] = {
	{"OpenGfxCard", OpenGfxCard},
	{NULL, NULL}    /* End of definition */
};

void initDRMCairo(lua_State *L){
	libSel_libFuncs( L, "DRMCairo", DRMCLib );
}

#endif
