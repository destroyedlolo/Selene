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

#include <sys/eventfd.h>

#define SO_LOCK 1
#define SO_NO_LOCK 0

struct _SharedStuffs SharedStuffs;


	/*******
	 * Shared variables
	 *******/

static struct SharedVar *findVar(const char *vn, int lock){
/* Find a variable
 * vn -> Variable name
 * lock -> lock (!=0) or not the variable
 */
	int aH = SelL_hash(vn);	/* get the hash of the variable name */
	struct SharedVar *v;

	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	for(v = SharedStuffs.first_shvar; v; v=v->succ){
		if(v->name.H == aH && !strcmp(v->name.name, vn)){
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
 * If it exists, the variable is freed.
 * If it doesn't exist, the variable is created
 */
	struct SharedVar *v = findVar(vname, SO_LOCK);
	
	if(v){	/* The variable already exists */
		if(v->type == SOT_STRING && v->val.str)	/* Free previous allocation */
			free( (void *)v->val.str );
		v->type = SOT_UNKNOWN;
	} else {	/* New variable */
		assert( (v = malloc(sizeof(struct SharedVar))) );
		assert( (v->name.name = strdup(vname)) );
		v->name.H = SelL_hash(vname);
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
/* get shared variable content */
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = findVar(vname, SO_LOCK);

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
	struct SharedVar *v = findVar(vname, SO_LOCK);
	if(v){
		lua_pushinteger(L, v->mtime);
		pthread_mutex_unlock( &v->mutex );
		return 1;
	}
	return 0;
}

	/* C interface */

enum SharedObjType soc_gettype( const char *vname ){
	struct SharedVar *v = findVar(vname, SO_LOCK);

	if(v){
		switch(v->type){
		case SOT_STRING:
		case SOT_XSTRING:
			pthread_mutex_unlock( &v->mutex );
			return SOT_STRING;
		default:
			pthread_mutex_unlock( &v->mutex );
			return v->type;
		}
	}
	return SOT_UNKNOWN;
}

void soc_clear( const char *vname ){	
/* delete a variable */
	struct SharedVar *v = findVar(vname, SO_LOCK);

	if(v){
		if(v->type == SOT_STRING && v->val.str)	/* Free previous allocation */
			free( (void *)v->val.str );

		v->type = SOT_UNKNOWN;
		pthread_mutex_unlock( &v->mutex );
	}
}

void soc_sets( const char *vname, const char *s, unsigned long int ttl ){	/* C API to set a variable with a string */
	struct SharedVar *v = findFreeOrCreateVar(vname);

	v->type = SOT_STRING;
	assert( (v->val.str = strdup( s )) );

	if(ttl)
		v->death = time(NULL) + ttl;
	v->mtime = time(NULL);
	pthread_mutex_unlock( &v->mutex );
}

void soc_setn( const char *vname, double content, unsigned long int ttl ){	/* C API to set a variable with a string */
	struct SharedVar *v = findFreeOrCreateVar(vname);

	v->type = SOT_NUMBER;
	v->val.num = content;

	if(ttl)
		v->death = time(NULL) + ttl;
	v->mtime = time(NULL);
	pthread_mutex_unlock( &v->mutex );
}

enum SharedObjType soc_get( const char *vname, struct SharedVarContent *res ){
	struct SharedVar *v = findVar(vname, SO_LOCK);

	if(v){
		res->mtime = v->mtime;
		switch(v->type){
		case SOT_STRING:
		case SOT_XSTRING:
			assert(( res->val.str = strdup( v->val.str ) ));
			pthread_mutex_unlock( &v->mutex );
			return( res->type = SOT_STRING );
		default:
			res->val.num = v->val.num;
			pthread_mutex_unlock( &v->mutex );
			return( res->type = v->type );
		}
	}
	return( res->type = SOT_UNKNOWN );
}

void soc_free( struct SharedVarContent *res ){
	if(res->type == SOT_STRING)
		free((char *)res->val.str);
	res->type = SOT_UNKNOWN;	/* Avoid reuse */
}

	/******
	 *  shared functions
	 *
	 *  RegisterSharedFunction / LoadSharedFunction : 
	 *  	share functions across threads (run the func in another thread)
	 *  RegisterFunction / PushTask(ByRef) :
	 *  	task list
	 ******/

static struct elastic_storage **checkSelSharedFunc(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelSharedFunc");
	luaL_argcheck(L, r != NULL, 1, "'SelSharedFunc' expected");
	return (struct elastic_storage **)r;
}

struct readerdt {
	int somethingtoread;
	struct elastic_storage *func;
};

static const char *reader( lua_State *L, void *ud, size_t *size ){
	struct readerdt *tracking = (struct readerdt *)ud;

	if( !tracking->somethingtoread )	/* It's over */
		return NULL;

	*size = tracking->func->data_sz; /* Read everything at once */
	tracking->somethingtoread = 0;

	return tracking->func->data;
}

int loadsharedfunction(lua_State *L, struct elastic_storage *func){
	struct readerdt dt;
	dt.somethingtoread = 1;
	dt.func = func;

	return lua_load( L, reader, &dt, func->name ? func->name : "unnamed"
#if LUA_VERSION_NUM > 501
		, NULL
#endif
	);
}

int ssfc_dumpwriter(lua_State *L, const void *b, size_t size, void *s){
	(void)L;	/* Avoid a warning */
	if(!(EStorage_Feed(s, b, size) ))
		return 1;	/* Unable to allocate some memory */
	
	return 0;
}

static int ssf_registersharedfunc(lua_State *L){
	const char *name = NULL;
	struct elastic_storage **storage, *t;

	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Function needed as 1st argument of SelShared.RegisterFunction()");
		return 2;
	}

	if(lua_type(L, 2) == LUA_TSTRING ){	/* Named function */
		name = lua_tostring(L, 2);
		int H = SelL_hash(name);
		struct elastic_storage *s;
		for( s = SharedStuffs.shfunc; s; s=s->next ){
			if( (H = s->H) && !strcmp(name, s->name) ){	/* Already registered */
				assert( (storage = (struct elastic_storage **)lua_newuserdata(L, sizeof(struct elastic_storage *))) );
				luaL_getmetatable(L, "SelSharedFunc");
				lua_setmetatable(L, -2);	/* Remove arguments */
				*storage = s;
				return 1;
			}
		}
		lua_pop(L, 1);	/* Remove the string as the function must be at the top */
	}

		/* Allocate the new storage */
	assert( (t = (struct elastic_storage *)malloc(sizeof(struct elastic_storage))) );
	assert( EStorage_init(t) );
	
	if(name)
		assert( EStorage_SetName( t, name, &SharedStuffs.shfunc ) );

	if(lua_dump(L, ssfc_dumpwriter, t
#if LUA_VERSION_NUM > 501
		,1
#endif
	) != 0)
		return luaL_error(L, "unable to dump given function");
	lua_pop(L,1);	/* remove the function from the stack */

	storage = (struct elastic_storage **)lua_newuserdata(L, sizeof(struct elastic_storage *));
	assert( storage );
	luaL_getmetatable(L, "SelSharedFunc");
	lua_setmetatable(L, -2);
	*storage = t;

	return 1;
}

static int ssf_loadsharedfunc(lua_State *L){
	if(lua_type(L, 1) != LUA_TSTRING ){
		lua_pushnil(L);
		lua_pushstring(L, "String needed as 1st argument of SelShared.LoadSharedFunction()");
		return 2;
	}

		/* Lookup for function */
	const char *name = lua_tostring(L, 1);
	int H = SelL_hash(name);
	struct elastic_storage *s;
	for( s = SharedStuffs.shfunc; s; s=s->next ){
		if( (H = s->H) && !strcmp(name, s->name) ){	/* Function found */
			int err;
			if( (err = loadsharedfunction(L, s)) ){
				lua_pushnil(L);
				lua_pushstring(L, (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error");
				return 2;
			}
			return 1;	/* The function is on the stack */
		}
	}
	return 0;	/* Function not found */
}

static int ssf_tostring(lua_State *L){
	struct elastic_storage **s = checkSelSharedFunc(L);
	lua_pushstring(L, (*s)->data);
	return 1;
}

static int ssf_getname(lua_State *L){
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


	/******
	 *  Tasks
	 ******/

static int ssf_registerref(lua_State *L){
	/* Register a new reference
	 * -> 1/number : reference to be registered
	 * -> 2/name : reference's name
	 * <- false if a reference already exists with the same name
	 * 		or in case of error
	 */
	const char *name;
	int H;
	struct SharedFuncRef *r;

	if(lua_type(L, 1) != LUA_TNUMBER){
		lua_pushnil(L);
		lua_pushstring(L, "Reference needed as 1st argument of SelShared.RegisterRef()");
		return 2;
	}

	if(lua_type(L, 2) != LUA_TSTRING ){
		lua_pushnil(L);
		lua_pushstring(L, "Function needed as 1st argument of SelShared.RegisterFunctionRef()");
		return 2;
	}

	name = lua_tostring(L, 2);
	H = SelL_hash(name);

	pthread_mutex_lock( &SharedStuffs.mutex_sfr );
	for( r = SharedStuffs.shfuncref; r; r=r->next ){
		if( (H = r->H) && !strcmp(name, r->name) ){	/* Already registered */
			pthread_mutex_unlock( &SharedStuffs.mutex_sfr );
			lua_pop(L, 2);	/* Pop 2 arguments */
			lua_pushboolean (L, 0);
			return 1;
		}
	}

	assert( (r = (struct SharedFuncRef *)malloc(sizeof(struct SharedFuncRef))) );
	assert( (r->name = strdup(name)) );
	r->H = H;
	r->ref = lua_tointeger(L,1);
	r->next = SharedStuffs.shfuncref;
	SharedStuffs.shfuncref = r;

	pthread_mutex_unlock( &SharedStuffs.mutex_sfr );

	lua_pushboolean (L, 1);
	return 1;
}

static int ssf_findref(lua_State *L){
	/* Find a reference by its name
	 * -> 1/name : name of the reference
	 * <- reference if found or false if it doesn't exist
	 */
	const char *name;
	int H;
	struct SharedFuncRef *r;

	if(lua_type(L, 1) != LUA_TSTRING ){
		lua_pushnil(L);
		lua_pushstring(L, "Function needed as 1st argument of SelShared.RegisterFunctionRef()");
		return 2;
	}

	name = lua_tostring(L, 1);
	H = SelL_hash(name);

	pthread_mutex_lock( &SharedStuffs.mutex_sfr );
	for( r = SharedStuffs.shfuncref; r; r=r->next ){
		if( (H = r->H) && !strcmp(name, r->name) ){	/* Already registered */
			lua_pushinteger (L, r->ref);
			pthread_mutex_unlock( &SharedStuffs.mutex_sfr );
			return 1;
		}
	}

		/* Not found */
	pthread_mutex_unlock( &SharedStuffs.mutex_sfr );
	lua_pushboolean (L, 0);
	return 1;
}

static const struct ConstTranscode _TO[] = {
	{ "MULTIPLE", TO_MULTIPLE },
	{ "ONCE", TO_ONCE },
	{ "LAST", TO_LAST },
	{ NULL, 0 }
};

static int so_toconst(lua_State *L ){
	return findConst(L, _TO);
}

static int so_pushtask(lua_State *L){
/* Push a task to the waiting list
 * 1: function to push
 * 2: MULTIPLE/ONCE/LAST
 * 		default ONCE
 * 		bool : true ONCE / false MULTIPLE
 * 		number : const velue
 */
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
/* Push a task reference
 * Same arguments as pushtash()
 */
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
/* Register a function to lookup table
 * 1: function
 * <- reference id
 */
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

	/*****
	 * Collections
	 *****/

/* Find a timed collection
 * vn -> Variable name
 * lock -> lock (!=0) or not the collection
 * 	(as unlink variable collection are never deleted, I don't think yet locking
 * 	is needed here. More, it may create deadlock as push() is already protected)
 */
static struct SharedCollection *findCollection( const char *vn, int lock){
	int aH = SelL_hash(vn);	/* get the hash of the variable name */
	struct SharedCollection *c;

	pthread_mutex_lock( &SharedStuffs.mutex_collection );
	for(c = SharedStuffs.collections; c; c=c->next){
		if(c->name.H == aH && !strcmp(c->name.name, vn)){ /* Found it */
			if(lock)
				sel_shareable_lock(c->collection.shareable);

			pthread_mutex_unlock( &SharedStuffs.mutex_collection );
			return c;
		}
	}

		/* not found */
	pthread_mutex_unlock( &SharedStuffs.mutex_collection );
	return NULL;
}

static struct SelTimedCollection *findTimedCollection( const char *vn, int lock){
	struct SharedCollection *c = findCollection(vn, lock);
	
	if(!c)	/* Not found at all */
		return NULL;

	if(c->type != COLTYPE_TIMED){	/* Not the good kind */
		if(lock)	/* Need to unlock */
			sel_shareable_unlock(c->collection.shareable);
		return NULL;
	}

	return c->collection.timed;
}

static int so_registertimedcollection(lua_State *L){
/* Register a timed collection
 * 1: SelTimedCollection
 * 2: name
 * <- nil if a collection is already registered with this name
 * 	  true of successful
 */
	struct SelTimedCollection **col = checkSelTimedCollection(L);
	const char *name = luaL_checkstring(L, 2);

	if( findCollection( name, SO_NO_LOCK ) ){	/* does a collection already registered for this name */
		lua_pushnil(L);
		lua_pushstring(L, "A collection is already registered with the same name");
		return 2;
	}

	struct SharedCollection *nv = malloc( sizeof( struct SharedCollection) );
	if(!nv){
		lua_pushnil(L);
		lua_pushstring(L, "No memory");
		return 2;
	}

	nv->name.name = strdup( name );
	if(!nv->name.name){
		free(nv);
		lua_pushnil(L);
		lua_pushstring(L, "No memory");
		return 2;
	}
	nv->name.H = SelL_hash( name );
	nv->type = COLTYPE_TIMED;
	nv->collection.timed = *col;

	pthread_mutex_lock( &SharedStuffs.mutex_collection );
	nv->next = SharedStuffs.collections;
	SharedStuffs.collections = nv;
	pthread_mutex_unlock( &SharedStuffs.mutex_collection );

	lua_pushboolean( L, true );
	return 1;
}

static int so_retreivetimedcollection(lua_State *L){
/* Find out a registered timed collection
 * 1: name of the registered collection
 * <- SelTimedCollection or nil if not found
 */
	const char *name = luaL_checkstring(L, 1);
	struct SelTimedCollection *col = findTimedCollection( name, SO_NO_LOCK );
	struct SelTimedCollection **p;

	if(!col){
		lua_pushnil(L);
		lua_pushstring(L, "Timed collection not found");
		return 2;
	}

		/* Let's create an object in the State */
	p = (struct SelTimedCollection **)lua_newuserdata(L, sizeof(struct SelTimedCollection *));
	assert(p);
	*p = col;

	luaL_getmetatable(L, "SelTimedCollection");
	lua_setmetatable(L, -2);

	return 1;
}

	/*****
	 * Objects and library
	 *****/

void soc_dump(){
	struct SharedVar *v;
	struct elastic_storage *p;
	struct SharedFuncRef *r;
	int i;
	struct SharedCollection *c;

	pthread_mutex_lock( &SharedStuffs.mutex_shvar );
	printf("*D* Dumping variables list f:%p l:%p\n", SharedStuffs.first_shvar, SharedStuffs.last_shvar);
	for(v = SharedStuffs.first_shvar; v; v=v->succ){
		printf("*I* name:'%s' (h: %d) - %p prev:%p next:%p mtime:%s", v->name.name, v->name.H, v, v->prev, v->succ, ctime(&v->mtime));
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
	for( p = SharedStuffs.shfunc; p; p = p->next )
		printf("\t%p : '%s' (%d)\n", p, p->name, p->H );
	pthread_mutex_unlock( &SharedStuffs.mutex_sfl );

	printf("*D* Dumping  shared references list\n");
	pthread_mutex_lock( &SharedStuffs.mutex_sfr );
	for( r = SharedStuffs.shfuncref; r; r = r->next )
		printf("\t%p : '%s' (%d) -> %d\n", r, r->name, r->H, r->ref );
	pthread_mutex_unlock( &SharedStuffs.mutex_sfr );

	pthread_mutex_lock( &SharedStuffs.mutex_tl );
	printf("*D* Dumping pending tasks list : %d / %d\n", SharedStuffs.ctask, SharedStuffs.maxtask);
	if(SharedStuffs.maxtask)
		printf("\t");
	for(i=SharedStuffs.ctask; i<SharedStuffs.maxtask; i++)
		printf("%x ", SharedStuffs.todo[i % SO_TASKSSTACK_LEN]);
	pthread_mutex_unlock( &SharedStuffs.mutex_tl );
	if(SharedStuffs.maxtask)
		puts("");

	printf("*D* Dumping  shared Collection list\n");
	pthread_mutex_lock( &SharedStuffs.mutex_collection );
	for(c = SharedStuffs.collections; c; c=c->next)
		printf("*D*\tname:'%s' (h: %d) - col : %p (%02d)\n", c->name.name, c->name.H, c->collection.shareable, c->type);
	pthread_mutex_unlock( &SharedStuffs.mutex_collection );
}

static int so_dump(lua_State *L){
	soc_dump();
	return 0;
}

static const struct luaL_Reg SelSharedLib [] = {
#ifdef COMPATIBILITY
	{"set", so_set},
	{"get", so_get},
	{"getmtime", so_mtime},
#endif
	{"Set", so_set},
	{"Get", so_get},
	{"GetMtime", so_mtime},
	{"mtime", so_mtime},	/* alias */
	{"RegisterSharedFunction", ssf_registersharedfunc},
	{"RegisterRef", ssf_registerref},
	{"FindRef", ssf_findref},
	{"LoadSharedFunction", ssf_loadsharedfunc},
	{"RegisterFunction", so_registerfunc},
	{"TaskOnceConst", so_toconst},
	{"PushTask", so_pushtask},
	{"PushTaskByRef", so_pushtaskref},
	{"RegisterTimedCollection", so_registertimedcollection},
	{"RetrieveTimedCollection", so_retreivetimedcollection},
	{"dump", so_dump},
	{NULL, NULL}
};

int initSelShared(lua_State *L){
/* Create SelShared library 
 */

	libSel_libFuncs( L, "SelShared", SelSharedLib );	/* Associate object's methods */
	return 1;
}

void initG_SelShared(lua_State *L){
/* Create repository for all shared stuffs
 */

		/* Shared variables */
	SharedStuffs.first_shvar = SharedStuffs.last_shvar = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_shvar, NULL);
	SharedStuffs.shfunc = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_sfl, NULL);
	SharedStuffs.shfuncref = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_sfr, NULL);


		/* Functions lookup table & tasks */
	lua_newtable(L);
	lua_setglobal(L, FUNCREFLOOKTBL);

	SharedStuffs.ctask = SharedStuffs.maxtask = 0;
	pthread_mutex_init( &SharedStuffs.mutex_tl, NULL);
	if((SharedStuffs.tlfd = eventfd( 0, 0 )) == -1 ){
		perror("SelShared's eventfd()");
		exit(EXIT_FAILURE);
	}


		/* Collections */
	SharedStuffs.collections = NULL;
	pthread_mutex_init( &SharedStuffs.mutex_collection, NULL);
}
