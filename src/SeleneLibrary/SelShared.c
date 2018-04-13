/* SelShared.c
 *
 * This file contains all stuffs related to object shared by multiple threads
 *
 * 07/06/2015 LF : First version
 * 15/06/2015 LF : Add tasklist
 * 28/06/2015 LF : switch to evenfd instead of pthread condition
 * 11/11/2015 LF : Add TaskOnce enum
 * 20/01/2016 LF : Rename as SelShared
 * 16/04/2016 LF : Add TTL for variables
 * 28/05/2016 LF : Add mtime to variables
 *
 * 05/04/2018 LF : Move to Selene v4
 */

#include "SelShared.h"
#include "configuration.h"
#include "elastic_storage.h"

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


	/*******
	 * Shared variables functions
	 *******/

static struct SharedVar *findVar(const char *vn, int lock){
/* Find a variable
 * vn -> Variable name
 * lock -> lock (!=0) or not the variable
 */
	int aH = hash(vn);	/* get the hash of the variable name */

	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	for(struct SharedVar *v = SharedStuffs.first_shvar; v; v=v->succ){
		if(v->H == aH && !strcmp(v->name, vn)){
			if( v->death != (size_t)-1 ){
				double diff = difftime( v->death, time(NULL) );	/* Check if the variable is still alive */
				if(diff <= 0){	/* No ! */
					pthread_mutex_lock( &v->mutex );
					if(v->type == SOT_STRING)
						free((void *)v->val.str);
					v->type = SOT_UNKNOWN;
					pthread_mutex_unlock( &v->mutex );
				}
			}
			if(lock)
				pthread_mutex_lock( &v->mutex );
			pthread_mutex_unlock( &SharedStuffs.mutex_shvar );


			return v;
		}
	}
	pthread_mutex_unlock( &SharedStuffs.mutex_shvar );
	return NULL;
}

static struct SharedVar *findFreeOrCreateVar(const char *vname){
/* Look for 'vname' variable.
 * If it exists, the variable is free.
 * If it doesn't exist, the variable is created
 */
	struct SharedVar *v = findVar(vname, SO_VAR_LOCK);
	
	if(v){	/* The variable already exists */
		if(v->type == SOT_STRING && v->val.str){	/* Free previous allocation */
			free( (void *)v->val.str );
			v->type = SOT_UNKNOWN;
		}
	} else {	/* New variable */
		assert( (v = malloc(sizeof(struct SharedVar))) );
		assert( (v->name = strdup(vname)) );
		v->H = hash(vname);
		v->type = SOT_UNKNOWN;
		v->death = (time_t) -1;
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

	return v;
}

void soc_sets( const char *vname, const char *s ){	/* C API to set a variable with a string */
	struct SharedVar *v = findFreeOrCreateVar(vname);

	v->type = SOT_STRING;
	assert( (v->val.str = strdup( s )) );
	pthread_mutex_unlock( &v->mutex );
}

static int so_set(lua_State *L){
/* set a shared variable
 * 1 : the variable's name
 * 2 : value (string or number)
 * 3 : time to live in seconds (optional)
 */
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = findFreeOrCreateVar(vname);

	switch(lua_type(L, 2)){
	case LUA_TSTRING:
		v->type = SOT_STRING;
		assert( (v->val.str = strdup( lua_tostring(L, 2) )) );
		break;
	case LUA_TNUMBER:
		v->type = SOT_NUMBER;
		v->val.num = lua_tonumber(L, 2);
		break;
	case LUA_TNIL:
		break;
	default :
		pthread_mutex_unlock( &v->mutex );
		lua_pushnil(L);
		lua_pushstring(L, "Shared variable can be only a Number or a String");
#ifdef DEBUG
		printf("*E* '%s' : Shared variable can be only a Number or a String\n*I* '%s' is now invalid\n", v->name, v->name);
#endif
		return 2;
	}

	if(lua_type(L, 3) == LUA_TNUMBER)	/* This variable has a limited time life */
		v->death = time(NULL) + lua_tointeger( L, 3 );

	v->mtime = time(NULL);
	pthread_mutex_unlock( &v->mutex );

	return 0;
}

static int so_get(lua_State *L){
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = findVar(vname, SO_VAR_LOCK);
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
		return 1;
	}
	return 0;
}

static int so_mtime(lua_State *L){
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = findVar(vname, SO_VAR_LOCK);
	if(v){
		lua_pushinteger(L, v->mtime);
		pthread_mutex_unlock( &v->mutex );
		return 1;
	}
	return 0;
}

	/******
	 *  shared functions
	 ******/
static int writer(lua_State *L, const void *b, size_t size, void *s){
	(void)L;	/* Avoid a warning */
	if(!(EStorage_Feed(s, b, size) ))
		return 1;	/* Unable to allocate some memory */
	
	return 0;
}

static int so_registersharedfunc(lua_State *L){
	struct elastic_storage **storage;
	struct elastic_storage *t = (struct elastic_storage *)malloc(sizeof(struct elastic_storage));
	assert( t );
	assert( EStorage_init(t) );

	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Function needed as 1st argument of SelShared.RegisterFunction()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TSTRING ){
		assert( EStorage_SetName( t, lua_tostring(L, 2), &SharedStuffs.shfunc ) );
		lua_pop(L, 1);	/* Remove the string as the function must be at the top */
	}

	if(lua_dump(L, writer, t, 1) != 0)
		return luaL_error(L, "unable to dump given function");
	lua_pop(L,1);	/* remove the function from the stack */

	storage = (struct elastic_storage **)lua_newuserdata(L, sizeof(struct elastic_storage *));
	assert( storage );
	luaL_getmetatable(L, "SelSharedFunc");
	lua_setmetatable(L, -2);
	*storage = t;

	return 1;
}

	/*****
	 * Objects
	 *****/

static int so_dump(lua_State *L){
	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	printf("*D* Dumping variables list f:%p l:%p\n", SharedStuffs.first_shvar, SharedStuffs.last_shvar);
	for(struct SharedVar *v = SharedStuffs.first_shvar; v; v=v->succ){
		printf("*I* name:'%s' (h: %d) - %p prev:%p next:%p mtime:%s", v->name, v->H, v, v->prev, v->succ, ctime(&v->mtime));
		if( v->death != (time_t) -1){
			double diff = difftime( v->death, time(NULL) );
			if(diff > 0)
				printf("*I*\t%f second(s) to live\n", diff);
			else
				puts("*I*\tThis variable is dead");
		}
		switch(v->type){
		case SOT_UNKNOWN:
			puts("\tUnknown type or invalid variable");
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

	printf("*D* Dumping named shared functions list\n");
	pthread_mutex_lock( &SharedStuffs.mutex_sfl );
	for( struct elastic_storage *p = SharedStuffs.shfunc; p; p = p->next )
		printf("\t%p : '%s' (%d)\n", p, p->name, p->H );
	pthread_mutex_unlock( &SharedStuffs.mutex_sfl );

#ifdef NOT_YET
	pthread_mutex_lock( &SharedStuffs.mutex_tl );
	printf("*D* Dumping pending tasks list : %d / %d\n\t", SharedStuffs.ctask, SharedStuffs.maxtask);
	for(int i=SharedStuffs.ctask; i<SharedStuffs.maxtask; i++)
		printf("%x ", SharedStuffs.todo[i % SO_TASKSSTACK_LEN]);
	puts("");
	pthread_mutex_unlock( &SharedStuffs.mutex_tl );
#endif
	
	return 0;
}

static const struct luaL_Reg SelSharedLib [] = {
	{"set", so_set},
	{"get", so_get},
	{"getmtime", so_mtime},
	{"mtime", so_mtime},	/* alias */
	{"RegisterSharedFunction", so_registersharedfunc},
#ifdef NOT_YET
	{"TaskOnceConst", so_toconst},
	{"PushTask", so_pushtask},
	{"PushTaskByRef", so_pushtaskref},
#endif
	{"dump", so_dump},
	{NULL, NULL}
};

int initSelShared(lua_State *L){
	libSel_libFuncs( L, "SelShared", SelSharedLib );	/* Associate object's methods */
	return 1;
}

int initSelSharedFunc(lua_State *L){
	libSel_objFuncs( L, "SelSharedFunc", NULL );	/* Create a meta table for shared functions */
	return 1;
}

void init_sharedRepo(lua_State *L){
	SharedStuffs.first_shvar = SharedStuffs.last_shvar = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_shvar, NULL);
	SharedStuffs.shfunc = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_sfl, NULL);

#ifdef NOT_YET
	SharedStuffs.ctask = SharedStuffs.maxtask = 0;
	pthread_mutex_init( &SharedStuffs.mutex_tl, NULL);
	if((SharedStuffs.tlfd = eventfd( 0, 0 )) == -1 ){
		perror("SelShared's eventfd()");
		exit(EXIT_FAILURE);
	}
#endif

}
