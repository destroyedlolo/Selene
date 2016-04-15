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

int GetDeviceDescription( lua_State *L ){
	DFBGraphicsDeviceDescription desc;
	DFBResult err;

	if((err = dfb->GetDeviceDescription(dfb, &desc)) != DFB_OK){
		lua_pushnil(L);
		lua_pushstring(L, DirectFBErrorString(err));
		return 2;
	}

	int i = desc.acceleration_mask;
	lua_pushinteger(L, i);
	i = desc.blitting_flags;
	lua_pushinteger(L, i);
	i = desc.drawing_flags;
	lua_pushinteger(L, i);

	return 3;
}

	/****
	 * C access functions
	 ****/
static void clean_directFB(void){
	dfb->Release( dfb );
}

void init_directfb(lua_State *L){
	const char *t[] = { "fake", lua_tostring(L, -1) };
	char **av = (char **)t;	/* VERY DANGEROUS but seems to be safe as of 1.7.6 */
	int ac = av[1] ? 2:1;


	DFBResult err = DirectFBInit(&ac, &av);
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
