/* sharedobj.c
 *
 * This file contains all stuffs related to object shared by multiple threads
 *
 * 07/06/2015 LF : First version
 * 15/06/2015 LF : Add tasklist
 * 28/06/2015 LF : switch to evenfd instead of pthread condition
 */

#include "sharedobj.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>		/* uint64_t */
#include <sys/eventfd.h>
#include <unistd.h>

#define SO_VAR_LOCK 1
#define SO_NO_VAR_LOCK 0

struct _SharedStuffs SharedStuffs;

	/*
	 * Shared variables function
	 */
static int hash( const char *s ){	/* Calculate the hash code of a string */
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
	int aH = hash(vn);	/* get the hash of the variable name */

	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	for(struct SharedVar *v = SharedStuffs.first_shvar; v; v=v->succ){
		if(v->H == aH && !strcmp(v->name, vn)){
			if(lock)
				pthread_mutex_lock( &v->mutex );
			pthread_mutex_unlock( &SharedStuffs.mutex_shvar );
			return v;
		}
	}
	pthread_mutex_unlock( &SharedStuffs.mutex_shvar );
	return NULL;
}


	/*
	 * Tasklist functions
	 */
int pushtask( int funcref, int once ){
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

	if(once){
		for(int i=SharedStuffs.ctask; i<SharedStuffs.maxtask; i++)
			if(SharedStuffs.todo[i] == funcref){	/* Already in the stack ... Ignoring */
				write( SharedStuffs.tlfd, &v, sizeof(v));
				pthread_mutex_unlock( &SharedStuffs.mutex_tl );

				return 0;
			}
	}

	if( SharedStuffs.maxtask - SharedStuffs.ctask >= SO_TASKSSTACK_LEN ){	/* Task is full */
		write( SharedStuffs.tlfd, &v, sizeof(v));	/* even if our task is not added, unlock others to try to resume this loosing condition */
		pthread_mutex_unlock( &SharedStuffs.mutex_tl );
		return( errno = EUCLEAN );
	}

	SharedStuffs.todo[ SharedStuffs.maxtask++ % SO_TASKSSTACK_LEN ] = funcref;

	write( SharedStuffs.tlfd, &v, sizeof(v));
	pthread_mutex_unlock( &SharedStuffs.mutex_tl );

	return 0;
}

	/*
	 * Lua functions
	 */
static int so_dump(lua_State *L){
	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	printf("List f:%p l:%p\n", SharedStuffs.first_shvar, SharedStuffs.last_shvar);
	for(struct SharedVar *v = SharedStuffs.first_shvar; v; v=v->succ){
		printf("*D*%p p:%p s:%p n:'%s' (%d)\n", v, v->prev, v->succ, v->name, v->H);
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
	pthread_mutex_unlock( &SharedStuffs.mutex_shvar );

	pthread_mutex_lock( &SharedStuffs.mutex_tl );
	printf("Pending tasks : %d / %d\n\t", SharedStuffs.ctask, SharedStuffs.maxtask);
	for(int i=SharedStuffs.ctask; i<SharedStuffs.maxtask; i++)
		printf("%x ", SharedStuffs.todo[i]);
	puts("");
	pthread_mutex_unlock( &SharedStuffs.mutex_tl );
	
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
		v->H = hash(vname);
		v->type = SOT_UNKNOWN;
		pthread_mutex_init(&v->mutex,NULL);
		pthread_mutex_lock( &v->mutex );

			/* Insert this new variable in the list */
		pthread_mutex_lock( &SharedStuffs.mutex_shvar );
		if(SharedStuffs.last_shvar){	/* the list is not empty */
			SharedStuffs.last_shvar->succ = v;
			v->prev = SharedStuffs.last_shvar;
		} else {	/* First in the list */
			SharedStuffs.first_shvar = v;
			v->prev = NULL;
		}
		SharedStuffs.last_shvar = v;
		v->succ = NULL;
		pthread_mutex_unlock( &SharedStuffs.mutex_shvar );
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
		lua_remove( L, 1 );	/* remove arguments */
		lua_remove( L, 2 );
		lua_pushnil(L);
		lua_pushstring(L, "Shared variable can be only an Integer or a String");
		printf("*E* '%s' : Shared variable can be only an Integer or a String\n'%s' is now invalid\n", v->name, v->name);
		return 2;
	}
	pthread_mutex_unlock( &v->mutex );

	lua_remove( L, 1 );	/* Remove arguments */
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
		pthread_mutex_unlock( &v->mutex );
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
	SharedStuffs.first_shvar = SharedStuffs.last_shvar = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_shvar, NULL);

	SharedStuffs.ctask = SharedStuffs.maxtask = 0;
	pthread_mutex_init( &SharedStuffs.mutex_tl, NULL);
	if((SharedStuffs.tlfd = eventfd( 0, 0 )) == -1 ){
		perror("SelShared's eventfd()");
		exit(EXIT_FAILURE);
	}

	init_shared_Lua(L);
}

