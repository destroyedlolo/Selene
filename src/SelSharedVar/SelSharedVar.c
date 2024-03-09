/* SelSharedVar.c
 *
 * Variable shared among threads
 *
 * 05/03/2024 First version
 */

#include "sharedvar.h"

#include <Selene/SelSharedVar.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

static struct SelSharedVar selSharedVar;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SharedVar *first_shvar, *last_shvar;
static pthread_mutex_t mutex_shvar;

static struct SharedVar *ssvc_findVar(const char *vn, bool lock){
/**
 * @brief Find a variable
 *
 * @function findVar
 * @tparam const char *vn Variable name
 * @tparam boolean lock lock or not the variable
 */
	int aH = selL_hash(vn);	/* get the hash of the variable name */
	struct SharedVar *v;

	pthread_mutex_lock( &mutex_shvar );
	for(v = first_shvar; v; v=v->succ){
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
			pthread_mutex_unlock( &mutex_shvar );


			return v;
		}
	}
	pthread_mutex_unlock( &mutex_shvar );
	return NULL;
}

static struct SharedVar *ssvc_findFreeOrCreateVar(const char *vname){
/**
 * @brief Empty a variable if it exists or create it
 *
 * @function findVar
 * @tparm const char *vname Variable name
 */
	struct SharedVar *v = ssvc_findVar(vname, true);
	
	if(v){	/* The variable already exists */
		if(v->type == SOT_STRING && v->val.str)	/* Free previous allocation */
			free( (void *)v->val.str );
		v->type = SOT_UNKNOWN;
	} else {	/* New variable */
		assert( (v = malloc(sizeof(struct SharedVar))) );
		assert( (v->name.name = strdup(vname)) );
		v->name.H = selL_hash(vname);
		v->type = SOT_UNKNOWN;
		v->death = (time_t) -1;
		pthread_mutex_init(&v->mutex, NULL);
		pthread_mutex_lock(&v->mutex);

			/* Insert this new variable in the list */
		pthread_mutex_lock( &mutex_shvar );
		if(last_shvar){	/* the list is not empty */
			last_shvar->succ = v;
			v->prev = last_shvar;
		} else {	/* First in the list */
			first_shvar = v;
			v->prev = NULL;
		}
		last_shvar = v;
		v->succ = NULL;
		pthread_mutex_unlock(&mutex_shvar);
	}

	return v;
}

static void ssvc_free(struct SharedVar *res){	/* release variable's content */
	if(res->type == SOT_STRING)
		free((char *)res->val.str);
	res->type = SOT_UNKNOWN;
}

static void ssvc_clear(const char *vname){
/**
 * @brief Clear a variable
 *
 * @function clear
 * @tparam const char * Variable name
 */
	struct SharedVar *v = ssvc_findVar(vname, true);
	if(v){
		ssvc_free(v);
		pthread_mutex_unlock(&v->mutex);
	}
}

static void ssvc_dump(){
	struct SharedVar *v;

	pthread_mutex_lock(&mutex_shvar);

	selLog->Log('D', "Dumping variables list f:%p l:%p", first_shvar, last_shvar);
	for(v = first_shvar; v; v=v->succ){
		selLog->Log('I', "name:'%s' (h: %d) - %p prev:%p next:%p mtime:%s", v->name.name, v->name.H, v, v->prev, v->succ, selCore->ctime(&v->mtime, NULL, 0));

		if(v->death != (time_t) -1){
			double diff = difftime(v->death, time(NULL));
			if(diff > 0)
				selLog->Log('I', "\t%f second(s) to live", diff);
			else
				selLog->Log('I', "\tThis variable is dead");
		}

		switch(v->type){
		case SOT_UNKNOWN:
			selLog->Log('I', "\tUnknown type or unset variable");
			break;
		case SOT_NUMBER:
			selLog->Log('I', "\tNumber : %lf", v->val.num);
			break;
		case SOT_STRING:
			selLog->Log('I', "\tDString : '%s'", v->val.str);
			break;
		case SOT_XSTRING:
			selLog->Log('I', "\tXString : '%s'", v->val.str);
			break;
		default :
			selLog->Log('E', "Unexpected type %d", v->type);
		}
	}

	pthread_mutex_unlock(&mutex_shvar);
}

static void ssvc_setn(const char *vname, double content, unsigned long int ttl){
/**
 * @brief Set a variable to a number
 *
 * @function setNumber
 * @tparam const char * Variable name
 * @tparam double content to put in the variable
 * @tparam unsigned long int time to live (or 0 for immortal)
 */
	struct SharedVar *v = ssvc_findFreeOrCreateVar(vname);

	v->type = SOT_NUMBER;
	v->val.num = content;

	if(ttl)
		v->death = time(NULL) + ttl;
	v->mtime = time(NULL);
	pthread_mutex_unlock(&v->mutex);
}

static void ssvc_sets(const char *vname, const char *content, unsigned long int ttl){
/**
 * @brief Set a variable to a number
 *
 * @function setString
 * @tparam const char * Variable name
 * @tparam const char * content to put in the variable
 * @tparam unsigned long int time to live (or 0 for immortal)
 */
	struct SharedVar *v = ssvc_findFreeOrCreateVar(vname);

	v->type = SOT_STRING;
	assert( (v->val.str = strdup(content)) );

	if(ttl)
		v->death = time(NULL) + ttl;
	v->mtime = time(NULL);
	pthread_mutex_unlock(&v->mutex);
}

static enum SharedObjType ssvc_getType(const char *vname){
	struct SharedVar *v = ssvc_findVar(vname, true);

	if(v){
		switch(v->type){
		case SOT_STRING:
		case SOT_XSTRING:
			pthread_mutex_unlock(&v->mutex);
			return SOT_STRING;
		default:
			pthread_mutex_unlock(&v->mutex);
			return v->type;
		}
	}
	return SOT_UNKNOWN;
}

static union SelSharedVarContent ssvc_getValue(const char *vname, enum SharedObjType *type, bool lock){
/**
 * @brief Get SelSharedVariableContent
 *
 * @function getValue
 * @tparam const char * Variable name
 * @tparam enum SharedObjType * type of the variable
 * @tparam bool lock do we have to lock the variable
 */
	struct SharedVar *v = ssvc_findVar(vname, lock);

	if(!v){
		*type = SOT_UNKNOWN;
		return (union SelSharedVarContent)0.0;
	}

	*type = v->type;
	return v->val;
}

static void ssvc_unlockVariable(const char *vname){
/**
 * @brief Unlock a variable locked by getValue()
 *
 * @function unlockVariable
 * @tparam const char * Variable name
 */
	struct SharedVar *v = ssvc_findVar(vname, false);	/* false MANDATORY to avoid deadlock */
	if(v)
		pthread_mutex_unlock(&v->mutex);
}

	/* ***
	 * Lua
	 * ***/

static int ssvl_dump(lua_State *L){
	ssvc_dump();
	return 0;
}

static int ssvl_set(lua_State *L){
/**
 * set a shared variable
 *
 * @function Set
 *
 * @tparam string name the variable's name
 * @tparam ?string|number value
 * @tparam number ttl time to live in seconds (optional)
 */
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = ssvc_findFreeOrCreateVar(vname);

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
		pthread_mutex_unlock(&v->mutex);
		lua_pushnil(L);
		lua_pushstring(L, "Shared variable can be only a Number or a String");
#ifdef DEBUG
		selLog->Log('E', "'%s' : Shared variable can be only a Number or a String", v->name);
		selLog->Log('I', "'%s' is now invalid", v->name);
#endif
		return 2;
	}

	if(lua_type(L, 3) == LUA_TNUMBER)	/* This variable has a limited time life */
		v->death = time(NULL) + lua_tointeger( L, 3 );

	v->mtime = time(NULL);
	pthread_mutex_unlock( &v->mutex );

	return 0;
}

static int ssvl_get(lua_State *L){
/**
 * Get a shared variable's content
 *
 * @function Get
 *
 * @tparam string name the variable's name
 * @treturn ?string|number|nil content of the variable
 */
	const char *vname = luaL_checkstring(L, 1);	/* Name of the variable to retrieve */
	struct SharedVar *v = ssvc_findVar(vname, true);

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
		pthread_mutex_unlock(&v->mutex);
		return 1;
	}
	return 0;
}

static const struct luaL_Reg SelSharedVarLib [] = {
	{"dump", ssvl_dump},
	{"Set", ssvl_set},
	{"Get", ssvl_get},
#if 0
	{"GetMtime", so_mtime},
	{"mtime", so_mtime},	/* alias */
#endif
	{NULL, NULL}
};

static void registerSelSharedVar(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelSharedVar", SelSharedVarLib);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Not mandatory as may be used by C code */
	selLua =  (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selSharedVar, "SelSharedVar", SELSHAREDVAR_VERSION, LIBSELENE_VERSION))
		return false;

	selSharedVar.module.dump = ssvc_dump;
	selSharedVar.clear = ssvc_clear;
	selSharedVar.setNumber = ssvc_setn;
	selSharedVar.setString = ssvc_sets;
	selSharedVar.getType = ssvc_getType;
	selSharedVar.getValue = ssvc_getValue;
	selSharedVar.unlockVariable = ssvc_unlockVariable;

	registerModule((struct SelModule *)&selSharedVar);

	pthread_mutex_init(&mutex_shvar, NULL);

	if(selLua){	/* Only if Lua is used */
		selLua->libCreateOrAddFuncs(NULL, "SelSharedVar", SelSharedVarLib);
		selLua->AddStartupFunc(registerSelSharedVar);
	}
#ifdef DEBUG
	else
		selLog->Log('D', "SelLua not loaded");
#endif

	return true;
}
