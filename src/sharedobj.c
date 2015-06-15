/* sharedobj.c
 *
 * This file contains all stuffs related to object shared by multiple threads
 *
 * 07/06/2015 LF : First version
 */

#include "sharedobj.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define SO_VAR_LOCK 1
#define SO_NO_VAR_LOCK 0

static struct {
	struct SharedVar *first, *last;
	pthread_mutex_t mutex;	/*AF* As long their is only 2 threads, a simple mutex is enough */
} GlobalVarList;

static int crc( const char *s ){
	int r = 0;
	for(; *s; s++)
		r += *s;
	return r;
}

static struct SharedVar *findvar(const char *vn, int lock){
/* Find a variable
 * vn -> Variable name
 * lock -> lock (!=0) or not the variable
 */
	int acrc = crc(vn);	/* get the crc of the variable name */

	pthread_mutex_lock( &GlobalVarList.mutex );
	for(struct SharedVar *v = GlobalVarList.first; v; v=v->succ){
		if(v->crc == acrc && !strcmp(v->name, vn)){
			if(lock)
				pthread_mutex_lock( &v->mutex );
			pthread_mutex_unlock( &GlobalVarList.mutex );
			return v;
		}
	}
	pthread_mutex_unlock( &GlobalVarList.mutex );
	return NULL;
}

	/* Lua functions */
static int so_dump(lua_State *L){
	pthread_mutex_lock( &GlobalVarList.mutex );

	printf("List f:%p l:%p\n", GlobalVarList.first, GlobalVarList.last);
	for(struct SharedVar *v = GlobalVarList.first; v; v=v->succ){
		printf("*D*%p p:%p s:%p n:'%s' (%d)\n", v, v->prev, v->succ, v->name, v->crc);
		switch(v->type){
		case SOT_UNKNOWN:
			puts("\tUnknown type");
			break;
		case SOT_NUMBER:
			printf("\tNumber : %lf\n", v->val.num);
			break;
		case SOT_STRING:
			printf("\tDString : '%s'\n", v->val.str);
			break;
		case SOT_XSTRING:
			printf("\tXString : '%s'\n", v->val.str);
			break;
		default :
			printf("*E* Unexpected type %d\n", v->type);
		}
	}

	pthread_mutex_unlock( &GlobalVarList.mutex );
	return 0;
}

static int so_set(lua_State *L){
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = findvar(vname, SO_VAR_LOCK);

	if(v){	/* The variable already exists */
		if(v->type == SOT_STRING && v->val.str)	/* Free previous allocation */
			free( (void *)v->val.str );
	} else {	/* New variable */
		assert( v = malloc(sizeof(struct SharedVar)) );
		assert( v->name = strdup(vname) );
		v->crc = crc(vname);
		v->type = SOT_UNKNOWN;
		pthread_mutex_init(&v->mutex,NULL);
		pthread_mutex_lock( &v->mutex );

			/* Insert this new variable in the list */
		pthread_mutex_lock( &GlobalVarList.mutex );
		if(GlobalVarList.last){	/* the list is not empty */
			GlobalVarList.last->succ = v;
			v->prev = GlobalVarList.last;
		} else {	/* First in the list */
			GlobalVarList.first = v;
			v->prev = NULL;
		}
		GlobalVarList.last = v;
		v->succ = NULL;
		pthread_mutex_unlock( &GlobalVarList.mutex );
	}

	switch(lua_type(L, 2)){
	case LUA_TSTRING:
		v->type = SOT_STRING;
		assert( v->val.str = strdup( lua_tostring(L, 2) ) );
		break;
	case LUA_TNUMBER:
		v->type = SOT_NUMBER;
		v->val.num = lua_tonumber(L, 2);
		break;
	default :
		pthread_mutex_unlock( &v->mutex );
		lua_remove( L, 1 );
		lua_remove( L, 2 );
		lua_pushnil(L);
		lua_pushstring(L, "Shared variable can be only an Integer or a String");
		printf("*E* '%s' : Shared variable can be only an Integer or a String\n'%s' is now invalid\n", v->name, v->name);
		return 2;
	}
	pthread_mutex_unlock( &v->mutex );

	lua_remove( L, 1 );
	lua_remove( L, 2 );
	return 0;
}

static int so_get(lua_State *L){
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = findvar(vname, SO_VAR_LOCK);
	if(v){
		switch(v->type){
		case SOT_STRING:
		case SOT_XSTRING:
			lua_pushstring(L, v->val.str);
			break;
		case SOT_NUMBER:
			lua_pushnumber(L, v->val.num);
			break;
		default :
			lua_pushnil(L);
			break;
		}
	}

	return 1;
}

static const struct luaL_reg SelSharedLib [] = {
	{"set", so_set},
	{"get", so_get},
	{"dump", so_dump},
	{NULL, NULL}
};


void init_shared_Lua(lua_State *L){
	luaL_newmetatable(L, "SelShared");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L,"SelShared", SelSharedLib);
}

void init_shared(lua_State *L){
	GlobalVarList.first = GlobalVarList.last = NULL;
	pthread_mutex_init( &GlobalVarList.mutex, NULL);

	init_shared_Lua(L);
}

