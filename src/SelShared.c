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
 */

#include "SelShared.h"

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

static const struct ConstTranscode _TO[] = {
	{ "MULTIPLE", TO_MULTIPLE },
	{ "ONCE", TO_ONCE },
	{ "LAST", TO_LAST },
	{ NULL, 0 }
};

static int so_toconst(lua_State *L ){
	return findConst(L, _TO);
}

	/*
	 * Shared variables function
	 */
static int hash( const char *s ){	/* Calculate the hash code of a string */
	int r = 0;
	for(; *s; s++)
		r += *s;
	return r;
}

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


	/*
	 * Tasklist functions
	 */
#ifdef DEBUG
int _pushtask
#else
int pushtask
#endif
( int funcref, enum TaskOnce once ){
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
		for(unsigned int i=SharedStuffs.ctask; i<SharedStuffs.maxtask; i++)
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

#ifdef DEBUG
int pushtask( int funcref, enum TaskOnce once ){
	puts("*d* pushtask ...");
	int r=_pushtask( funcref, once );
	puts("*d* pushtask : ok");
	return r;
}
#endif

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
		int func = luaL_checkint(L, -1);
		lua_pop(L, 1);	/* Pop the reference */
		return func;
	}
}

	/*
	 * Lua functions
	 */
static int so_pushtask(lua_State *L){
	enum TaskOnce once = TO_ONCE;
	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Task needed as 1st argument of SelShared.PushTask()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TBOOLEAN )
		once = lua_toboolean(L, 2) ? TO_ONCE : TO_MULTIPLE;
	else if( lua_type(L, 2) == LUA_TNUMBER )
		once = lua_tointeger(L, 2);

	int err = pushtask( findFuncRef(L,1), once);
	if(err){
		lua_pushnil(L);
		lua_pushstring(L, strerror(err));
		return 2;
	}

	return 0;
}

static int so_pushtaskref(lua_State *L){
	enum TaskOnce once = TO_ONCE;
	if(lua_type(L, 1) != LUA_TNUMBER){
		lua_pushnil(L);
		lua_pushstring(L, "Task reference needed as 1st argument of SelShared.PushTaskByRef()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TBOOLEAN )
		once = lua_toboolean(L, 2) ? TO_ONCE : TO_MULTIPLE;
	else if( lua_type(L, 2) == LUA_TNUMBER )
		once = lua_tointeger(L, 2);

	int err = pushtask( lua_tointeger(L, 1), once);
	if(err){
		lua_pushnil(L);
		lua_pushstring(L, strerror(err));
		return 2;
	}

	return 0;
}

static int so_registerfunc(lua_State *L){
	lua_getglobal(L, FUNCREFLOOKTBL);	/* Check if this function is already referenced */
	if(!lua_istable(L, -1)){
		fputs("*F* GetTaskID can be called only by the main thread\n", stderr);
		exit(EXIT_FAILURE);
	}
	lua_pop(L,1);

	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Task needed as 1st argument of SelShared.RegisterFunction()");
		return 2;
	}

	lua_pushinteger(L, findFuncRef(L,1));
	return 1;
}

static int so_dump(lua_State *L){
	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	printf("List f:%p l:%p\n", SharedStuffs.first_shvar, SharedStuffs.last_shvar);
	for(struct SharedVar *v = SharedStuffs.first_shvar; v; v=v->succ){
		printf("*D* %p p:%p s:%p n:'%s' (h: %d) mtime:%s\n", v, v->prev, v->succ, v->name, v->H, ctime(&v->mtime));
		if( v->death != (time_t) -1){
			double diff = difftime( v->death, time(NULL) );
			if(diff > 0)
				printf("*D* %f second(s) to live\n", diff);
			else
				puts("*D* This variable is dead\n");
		}
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
		printf("%x ", SharedStuffs.todo[i % SO_TASKSSTACK_LEN]);
	puts("");
	pthread_mutex_unlock( &SharedStuffs.mutex_tl );
	
	return 0;
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
		assert( v = malloc(sizeof(struct SharedVar)) );
		assert( v->name = strdup(vname) );
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
	assert( v->val.str = strdup( s ) );
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
		assert( v->val.str = strdup( lua_tostring(L, 2) ) );
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
		printf("*E* '%s' : Shared variable can be only a Number or a String\n*I* '%s' is now invalid\n", v->name, v->name);
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

static const struct luaL_reg SelSharedLib [] = {
	{"set", so_set},
	{"get", so_get},
	{"getmtime", so_mtime},
	{"mtime", so_mtime},	/* alias */
	{"dump", so_dump},
	{"RegisterFunction", so_registerfunc},
	{"TaskOnceConst", so_toconst},
	{"PushTask", so_pushtask},
	{"PushTaskByRef", so_pushtaskref},
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
	lua_newtable(L);	/* Create function lookup table */
	lua_setglobal(L, FUNCREFLOOKTBL);

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

