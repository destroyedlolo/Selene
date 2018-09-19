/* LuaSupportFunc
 *
 *	General Lua functions
 */

#include "libSelene.h"
#include "configuration.h"
#include "SelShared.h"
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <stdint.h>		/* uint64_t */
#include <sys/eventfd.h>
#include <unistd.h>

struct startupFunc {
	struct startupFunc *next;			/* Next entry */
	int (*func)( lua_State * );	/* Function to launch */
};

void *libSel_AddStartupFunc( void *lst, int (*func)( lua_State * ) ){
	struct startupFunc *new = malloc( sizeof(struct startupFunc) );
	if(!new)
		return NULL;

	new->func = func;
	new->next = lst;

	return new;
}

void libSel_ApplyStartupFunc( void *list, lua_State *L ){
	struct startupFunc *lst = (struct startupFunc *)list;	/* just to avoid zillion of casts */

	for(;lst; lst = lst->next)
		lst->func( L );
}

int libSel_libAddFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs){
	lua_getglobal(L, name);
	if(!lua_istable(L, -1)){
		fprintf(stderr, "Can't add functions to unknown library \"%s\"\n", name);
		return 0;
	}
#if LUA_VERSION_NUM > 501
	luaL_setfuncs (L, funcs, 0);
#else
	luaL_register(L, NULL, funcs);
#endif

	return 1;
}


int libSel_libFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs){
#if LUA_VERSION_NUM > 501
	lua_newtable(L);
	luaL_setfuncs (L, funcs, 0);
	lua_pushvalue(L, -1);	// pluck these lines out if they offend you
	lua_setglobal(L, name); // for they clobber the Holy _G
#else
	luaL_register(L, name, funcs);
#endif

	return 1;
}

int libSel_objFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs){
	luaL_newmetatable(L, name);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */

	if(funcs){	/* May be NULL if we're creating an empty metatable */
#if LUA_VERSION_NUM > 501
		luaL_setfuncs( L, funcs, 0);
#else
		luaL_register(L, NULL, funcs);
#endif
	}

	return 1;
}

int findConst( lua_State *L, const struct ConstTranscode *tbl ){
	const char *arg = luaL_checkstring(L, 1);	/* Get the constant name to retreave */
	unsigned int i;

	for(i=0; tbl[i].name; i++){
		if(!strcmp(arg, tbl[i].name)){
			lua_pushnumber(L, tbl[i].value);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushstring(L, arg);
	lua_pushstring(L," : Unknown constant");
	lua_concat(L, 2);
	return 2;
}

int rfindConst( lua_State *L, const struct ConstTranscode *tbl ){
	int arg = luaL_checkinteger(L, 1);	/* Get the integer to retrieve */
	unsigned int i;

	for(i=0; tbl[i].name; i++){
		if( arg == tbl[i].value ){
			lua_pushstring(L, tbl[i].name);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushinteger(L, arg);
	lua_tostring(L, -1);
	lua_pushstring(L," : Unknown constant");
	lua_concat(L, 2);
	return 2;
}

int findFuncRef(lua_State *L, int num){
	lua_getglobal(L, FUNCREFLOOKTBL);	/* Check if this function is already referenced */
	if(!lua_istable(L, -1)){
		fputs( FUNCREFLOOKTBL " not defined as a table\n", stderr);
		exit(EXIT_FAILURE);
	}
	lua_pushvalue(L, num);	/* The function is the key */
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)){	/* Doesn't exist yet */
		lua_pop(L, 1);	/* Remove nil */

		lua_pushvalue(L, num); /* Get its reference */
		int func = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_pushvalue(L, num); 		/* Push the function as key */
		lua_pushinteger(L, func);	/* Push it's reference */
		lua_settable(L, -3);

		lua_pop(L, 1);	/* Remove the table */
		return func;
	} else {	/* Reference already exists */
		lua_remove(L, -2);	/* Remove the table */
		int func = luaL_checkinteger(L, -1);
		lua_pop(L, 1);	/* Pop the reference */
		return func;
	}
}

int pushtask( int funcref, enum TaskOnce once ){
/* Push funcref in the stack
 * 	-> funcref : function reference as per luaL_ref
 * 	<- error code : 
 * 		0 = no error
 * 		EUCLEAN = stack full
 *
 *	in case of error, errno is set as well
 */
	uint64_t v = 1;
	pthread_mutex_lock( &SharedStuffs.mutex_tl );

	if(once != TO_MULTIPLE){
		unsigned int i;
		for(i=SharedStuffs.ctask; i<SharedStuffs.maxtask; i++)
			if(SharedStuffs.todo[i % SO_TASKSSTACK_LEN] == funcref){	/* Already in the stack */
				if(once == TO_LAST)	/* Put it at the end of the queue */
					SharedStuffs.todo[i % SO_TASKSSTACK_LEN] = LUA_REFNIL;	/* Remove previous reference */
				else {	/* TO_ONCE : Don't push a new one */
					write( SharedStuffs.tlfd, &v, sizeof(v));
					pthread_mutex_unlock( &SharedStuffs.mutex_tl );

					return 0;
				}
			}
	}

	if( SharedStuffs.maxtask - SharedStuffs.ctask >= SO_TASKSSTACK_LEN ){	/* Task is full */
		write( SharedStuffs.tlfd, &v, sizeof(v));	/* even if our task is not added, unlock others to try to resume this loosing condition */
		pthread_mutex_unlock( &SharedStuffs.mutex_tl );
		return( errno = EUCLEAN );
	}

	SharedStuffs.todo[ SharedStuffs.maxtask++ % SO_TASKSSTACK_LEN ] = funcref;

	if(!(SharedStuffs.ctask % SO_TASKSSTACK_LEN)){	/* Avoid counter to overflow */
		SharedStuffs.ctask %= SO_TASKSSTACK_LEN;
		SharedStuffs.maxtask %= SO_TASKSSTACK_LEN;
	}

	write( SharedStuffs.tlfd, &v, sizeof(v));
	pthread_mutex_unlock( &SharedStuffs.mutex_tl );

	return 0;
}

void initSeleneLibrary( lua_State *L ){
	initG_Selene();
	initG_SelShared(L);
	initG_SelLog();
	initG_SeleMQTT();
}

