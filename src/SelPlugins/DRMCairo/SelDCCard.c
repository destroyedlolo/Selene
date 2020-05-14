/* SelDCCard.c
 *
 * This file contains stuffs related to DRM card managment.
 *
 * 13/05/2020 LF : Creation
 *
 * sources :
 * 	https://waynewolf.github.io/2012/09/05/libdrm-samples/
 * 	https://events.static.linuxfound.org/sites/events/files/lcjpcojp13_pinchart.pdf
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

struct DCCard **checkSelDCCard(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelDCCard");
	luaL_argcheck(L, r != NULL, 1, "'SelDCCard' expected");
	return (struct DCCard **)r;
}

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

static int Open(lua_State *L){
	/* Initialise a card
	 * -> 1: card path (if not set /dev/dri/card0)
	 */
	const char *card = "/dev/dri/card0";

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
		struct DCCard *t = *q;
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

	return 1;
}

	/* Object itself functions */
static const struct luaL_Reg SelDCCardM[] = {
	{"GetSize", GetSize},
	{"CountAvailableModes", CountAvailableModes},
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
