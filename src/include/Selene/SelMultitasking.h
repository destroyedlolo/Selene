/* SelMultitasking.h
 *
 * Detached tasks
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELMULTITASKING_VERSION
#include <Selene/libSelene.h>
#include <Selene/SelElasticStorage.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELMULTITASKING_VERSION 1

#ifdef __cplusplus
extern "C"
{
#endif

struct SelMultitasking {
	struct SelModule module;

		/* Call backs */
	lua_State *(*createSlaveState)(void);
	bool (*loadandlaunch)( lua_State *L, lua_State *newL, struct elastic_storage *storage, int nargs, int nresults, int trigger, enum TaskOnce trigger_once);
	bool (*newthreadfunc)(lua_State *, struct elastic_storage *);
	int (*dumpwriter)(lua_State *, const void *, size_t, void *); /* need selElasticStorage */
};

#ifdef __cplusplus
}
#endif

#endif
