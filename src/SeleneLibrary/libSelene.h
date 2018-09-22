/*	libSelene.h
 *
 * 	Definitions shared outside library code itself
 */

#ifndef SEL_LIBRARY_H
#define SEL_LIBRARY_H	4.0701	/* libSelene version (major, minor, sub) */

#ifdef __cplusplus
extern "C"
{
#endif

#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */

#include <stdbool.h>

#include "elastic_storage.h"

/*
 *	Options agnostics libraries contents
 */

extern char *SelL_strdup( const char * );	/* May missing on some system */
extern int SelL_hash( const char * );	/* calculates hash code of a string */

/*
 * Lua startup functions management
 *
 *	These function will be called at every thread creation
 *	(Generally, for libraries initialisation)
 */
#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>	/* Functions to open libraries */

	/* This function **MUST** be called before any call to shared object
	 * stuffs.
	 * It has to be called only once, within the main Lua State
	 */
extern void initSeleneLibrary( lua_State *L );

	/* Creates Selene objects
	 * -> L : Lua State
	 */
extern int initSelene( lua_State *L );
extern int initReducedSelene( lua_State *L);
extern int initSelLog( lua_State *L );
extern int initSelCollection( lua_State *L );
extern int initSelTimedCollection( lua_State *L );
extern int initSelTimedWindowCollection( lua_State *L );
extern int initSelTimer( lua_State *L );
extern int initSelShared( lua_State *L );
extern int initSelSharedFunc( lua_State *L );
extern int initSelFIFO( lua_State *L );
extern int initSelEvent( lua_State * );
extern int initSelMQTT( lua_State * );
extern int initSeleMQTT( lua_State * );

	/* Add a function to a startup list
	 * -> func : startup function to call
	 * -> list : current list (returned from a previous libSel_AddStartupFunc()
	 *  call. Null for the 1st one.
	 * <- return an opaque list object (NULL in case of fatal error)
	 *
	 * Notez-bien : they will be called in reverse order
	 */
extern void *libSel_AddStartupFunc( void *list, int (*func)( lua_State * ) );

	/* Apply startup function to a Lua's State
	 * -> L : Lua state
	 * -> list : pointer returned by last libSel_AddStartupFunc() call
	 */
extern void libSel_ApplyStartupFunc( void *list, lua_State *L );

	/****************
	 * C interfaces *
	 ****************/

	/******
	 *  Lua libraries and objects management
	 ******/

	/* Add library's functions
	 * -> L : Lua State
	 * -> name : library name
	 * -> funcs : functions array
	 */
extern int libSel_libFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs);

	/* Add additional functions to an existing library
	 * -> L : Lua State
	 * -> name : library name
	 * -> funcs : functions array
	 */
extern int libSel_libAddFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs);

	/* Add object's functions
	 * -> L : Lua State
	 * -> name : library name
	 * -> funcs : functions array
	 */
extern int libSel_objFuncs( lua_State *L, const char *name, const struct luaL_Reg *funcs);

#if LUA_VERSION_NUM <= 501
extern void * luaL_testudata(lua_State *, int, const char *);
#endif

	/******
	 *  Trans codification stuffs 
	 ******/

struct ConstTranscode {
	const char *name;
	const int value;
};

extern int findConst( lua_State *, const struct ConstTranscode * );
extern int rfindConst( lua_State *, const struct ConstTranscode * );

	/****
	 * Shared functions 
	 ****/

enum SharedObjType {
	SOT_UNKNOWN = 0,	/* Invalid variable */
	SOT_NUMBER,		/* Integers */
	SOT_STRING,		/* Dynamically allocated string (managed by sharedobj' functions) */
	SOT_XSTRING		/* Const char * managed externally (constant, allocated elsewhere ...) */
};

/* get variable's type
 * 	-> name : variable's name
 * 	<- variable type (SOT_UNKNOWN if unset or dead)
 */
extern enum SharedObjType soc_gettype( const char *name );

/* set a string variables 
 *	-> name : variable's name
 *		content : value to put in
 *		ttl : if not null, live for ttl seconds
 */
extern void soc_sets( const char *name, const char *content, unsigned long int ttl);

/* set a number variables 
 *	-> name : variable's name
 *		content : value to put in
 *		ttl : if not null, live for ttl seconds
 */
extern void soc_setn( const char *name, double content, unsigned long int ttl);

/* get variable's content
 * -> name : variable's name
 *	res : struct that will hold the result
 * <- type of the variable
 *
 * Note : the content of the variable is copied into 'res'. 
 * Consequently, the variable is not locked, there is no risk of race condition
 * regarding the usage of the returned value.
 * 'res' has to be freed using soc_free() when not anymore used.
 */
struct SharedVarContent {
	enum SharedObjType type;
	union {
		double num;
		const char *str;
	} val;
	time_t mtime;	/* Time of the last modification */
};
extern enum SharedObjType soc_get( const char *name, struct SharedVarContent *res );
extern void soc_free( struct SharedVarContent * );	/* free return of soc_get() */

/* Mark a variable as invalid
 * -> vname : variable's name
 */
extern void soc_clear( const char *vname );

/* Find a function reference
 * -> L : Lua State
 * -> id : function identifier
 */
extern int findFuncRef(lua_State *L, int id);

/* Load a shared function from an elastic storage
 * -> L : Lua State
 * -> func : stored function to load
 * <- same as lua_load()'s return
 */
extern int loadsharedfunction(lua_State *L, struct elastic_storage *func);

/* Callback function to load a function from a state using lua_dump()
 * -> L : Lua State
 * -> dt : data to store
 * -> sz : size of the data to store
 * -> storage : where to store data (elastic_storage)
 * <- error code (0 neans no error)
 */
extern int ssfc_dumpwriter(lua_State *L, const void *dt, size_t sz, void *storage);

/* debug
 * dump shared stuffs to stdout
 */
extern void soc_dump();

	/****
	 * MQTT
	 ****/
#include <MQTTClient.h>	/* Paho MQTT library needed */

/* Compare 2 strings like strcmp() but s can contain MQTT wildcards
 * '#' : replace remaining of the line
 * '+' : a sub topic and must be enclosed by '/'
 *
 *  
 * Wildcards are checked as per mosquitto's source code rules
 * (comment in http://git.eclipse.org/c/mosquitto/org.eclipse.mosquitto.git/tree/src/subs.c)
 *
 * <- 0 : strings match
 * <- -1 : wildcard error
 * <- others : strings are different
 */
extern int mqtttokcmp(register const char *s, register const char *t);

/* Publish an MQTT message */
extern int mqttpublish(MQTTClient client, const char *topic, int length, void *payload, int retained );

/* Provide string corresponding to an MQTT error code
 * -> error code
 * <- the string corresponding
 */
extern const char *smqc_CStrError( int code );

/* Initialize SeleMQTT
 * -> client : already initialized MQTTClient object
 * -> clientID : pointer to used clientID.
 *  	NOTEZ-BIEN : the pointed string must exist as long as this SeleMQTT is used.
 */
extern void semc_initializeSeleMQTT( MQTTClient client, const char *clientID );

	/****
	 * logging
	 ****/
enum WhereToLog {
	LOG_DECIDEYOUSELF = 0,
	LOG_FILE = 1,
	LOG_STDOUT = 2
};

/* Register another logging level.
 * -> level : level letter
 * -> ext : topic extension
 * <- is the registering succeded
 *
 * Final topic is <Majordome_ID>/Log/<ext>
 */
extern bool slc_registerTransCo( const char level, const char *ext );

/* Initialise logging mechanism
 * -> filename : file to log to (open in append mode)
 * -> alogto : where to log to (bit flags are LOG_FILE and LOG_STDOUT)
 *		if NULL, log in STDOUT if interactive or to file else
 *	<- if false, see errno for failure reason.
 */
extern bool slc_init( const char *filename, enum WhereToLog alogto );

/* Initialise MQTT for logging
 * -> aClient : MQTT client
 * -> cID : Client ID
 *
 *	/!\ Caution : only a pointer to those data is kept, so the original
 *	objects must exist as long as MQTT is used for logging.
 *	Use slc_initMQTT(NULL, NULL); to disable
 */
extern void slc_initMQTT( MQTTClient aClient, const char *cID );

/* log a message
 * -> level 'F'atal, 'E'rror, 'W'arning otherwise 'I'nfo
 * -> msg : message to log
 *	<- if false, see errno for failure reason.
 */
extern bool slc_log( const char level, const char *msg );

#ifdef __cplusplus
}
#endif

#endif
