/***
Shared function.

Functions that can be called accross threads

 @classmod SelSharedFunc
*/

#include "SelShared.h"
#include "elastic_storage.h"

static struct elastic_storage **checkSelSharedFunc(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelSharedFunc");
	luaL_argcheck(L, r != NULL, 1, "'SelSharedFunc' expected");
	return (struct elastic_storage **)r;
}

static int ssf_tostring(lua_State *L){
/**
 * Return function's bytecode as string
 *
 * @function tostring
 *
 * @treturn string
 */
	struct elastic_storage **s = checkSelSharedFunc(L);
	lua_pushstring(L, (*s)->data);
	return 1;
}

static int ssf_getname(lua_State *L){
/**
 * Return function's Name
 *
 * @function getName
 *
 * @treturn string
 */
	struct elastic_storage **s = checkSelSharedFunc(L);
	lua_pushstring(L, (*s)->name);
	return 1;
}
static const struct luaL_Reg SelFuncSharedM [] = {
	{"tostring", ssf_tostring},
	{"getName", ssf_getname},
	{NULL, NULL}
};

int initSelSharedFunc(lua_State *L){
	libSel_objFuncs( L, "SelSharedFunc", SelFuncSharedM );	/* Create a meta table for shared functions */
	return 1;
}


