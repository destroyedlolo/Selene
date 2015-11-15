/* directfb.c
 *
 * This file contains all stuffs related to DirectFB.
 *
 * 13/04/2015 LF : First version
 */
#include "directfb.h"

#ifdef USE_DIRECTFB
#include <assert.h>

IDirectFB *dfb = NULL;	/* DirectDB's "super interface" */

	/*****
	 * Transcodification
	 *****/

static const struct ConstTranscode _CooperativeLevel[] = {
	{ "NORMAL", DFSCL_NORMAL },
	{ "FULLSCREEN", DFSCL_FULLSCREEN },
	{ "EXCLUSIVE", DFSCL_EXCLUSIVE },
	{ NULL, 0 }
};

int CooperativeConst( lua_State *L ){
	return findConst(L, _CooperativeLevel);
}

int SetCooperativeLevel( lua_State *L ){
	int arg = luaL_checkint(L, 1);	/* Get the constant name to retrieve */
	DFBResult err;
	assert(dfb);

	if((err = dfb->SetCooperativeLevel(dfb, arg)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}
	return 0;
}

	/****
	 * C access functions
	 ****/
static void clean_directFB(void){
	dfb->Release( dfb );
}

void init_directfb(lua_State *L, int *ac, char ***av ){
	DFBResult err = DirectFBInit(ac, av);
	if(err != DFB_OK){
		DirectFBError("DirectFBInit()", err);
		exit(EXIT_FAILURE);
	}

	if((err = DirectFBCreate (&dfb)) != DFB_OK){
		DirectFBError("DirectFBCreate()", err);
		exit(EXIT_FAILURE);
	}

	atexit(clean_directFB);

	_include_SelSurface(L);
	_include_SelScreen(L);
	_include_SelLayer(L);
	_include_SelWindow(L);
	_include_SelImage(L);
	_include_SelFont(L);
}
#endif
