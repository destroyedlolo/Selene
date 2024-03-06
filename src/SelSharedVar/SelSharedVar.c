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

struct SelSharedVar selSharedVar;


struct SeleneCore *selCore;
struct SelLog *selLog;
struct SelLua *selLua;

struct SharedVar *first_shvar, *last_shvar;
pthread_mutex_t mutex_shvar;

static const struct luaL_Reg SelSharedVarLib [] = {
/*	{"Set", sll_ignore}, */
	{NULL, NULL}
};

static void registerSelSharedVar(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelSharedVar", SelSharedVarLib);
}

static struct SharedVar *findVar(const char *vn, bool lock){
/**
 * @brief Find a variable
 *
 * @function findVar
 * @tparm const char *vn Variable name
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

static struct SharedVar *findFreeOrCreateVar(const char *vname){
/**
 * @brief Empty a variable if it exists or create it
 *
 * @function findVar
 * @tparm const char *vname Variable name
 */
	struct SharedVar *v = findVar(vname, true);
	
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
