/* SelScripting.c
 *
 * Expose Selene's Lua scripting functions.
 *
 * 08/02/2024 First version
 */
#include "Selene/SelScripting.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"
#include "Selene/SelLua.h"

#include <time.h>
#include <unistd.h>
#include <signal.h>

static struct SelScripting selScripting;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;


	/* ***
	 * Methods exposed to main threads only
	 * ***/
static int ssl_Use( lua_State *L ){
/** 
 * @brief Load a module
 *
 * @function Use
 * @tparam module_name Load a module
 * @treturn boolean does it succeed
 */
	uint16_t verfound;
	const char *name = luaL_checkstring(L, 1);

		/* No need to check for version as it only meaningful at C level */
	struct SelModule *m = selCore->loadModule(name, 0, &verfound, 'E');

	if(m){
		if(m->initLua)
			m->initLua();
		lua_pushboolean(L, 1);
	} else
		lua_pushboolean(L, 0);
	return 1;
}

	/*
	 * Signal handling
	 */

static int sigfunc = LUA_REFNIL;	/* Function to call in case of SIG_INT */

static void sighandler(){
/*
	if( sigfunc != LUA_REFNIL )
		pushtask( sigfunc, true );
*/
}

static int ssl_SigIntTask(lua_State *L){
/** 
 * Set **SIGINT** and **SIGUSR1** signal handler
 *
 * @function SigIntTask
 * @tparam function Function to be scheduled when a signal is received
 */
	if( lua_type(L, -1) == LUA_TFUNCTION ){
		sigfunc = selLua->findFuncRef(L,lua_gettop(L));

		signal(SIGINT, sighandler);
		signal(SIGUSR1, sighandler);
	}
	return 0;
}

static const struct luaL_Reg seleneExtLib[] = {	/* Extended ones */
/*
	{"WaitFor", SelWaitFor},
	{"Detach", SelDetach},
*/
	{"SigIntTask", ssl_SigIntTask},
	{"Use", ssl_Use},
	{NULL, NULL} /* End of definition */
};

	/* ***
	 * Methods exposed to slave threads as well 
	 * ***/

static int ssl_Sleep( lua_State *L ){
/** 
 * @brief Sleep some seconds.
 *
 * @function Sleep
 * @tparam num number of seconds to sleep
 */
	struct timespec ts;
	lua_Number lenght = luaL_checknumber(L, 1);
	ts.tv_sec = (time_t)lenght;
	ts.tv_nsec = (unsigned long int)((lenght - (time_t)lenght) * 1e9);

	nanosleep( &ts, NULL );
	return 0;
}

static int ssl_Hostname( lua_State *L ){
/** 
 * @brief Get the host's name.
 *
 * @function getHostname
 * @treturn string the host's name
 */
	char n[HOST_NAME_MAX];
	gethostname(n, HOST_NAME_MAX);

	lua_pushstring(L, n);
	return 1;
}

static int ssl_getPID( lua_State *L ){
/** 
 * @brief Get the current process ID
 *
 * @function getPid
 * @treturn num PID of the current process
 */
	lua_pushinteger(L, getpid());
	return 1;
}

static const struct luaL_Reg seleneLib[] = {
	{"Sleep", ssl_Sleep},
	{"Hostname", ssl_Hostname},
	{"getHostname", ssl_Hostname},
	{"getPid", ssl_getPID},
	{NULL, NULL} /* End of definition */
};

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
	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;


		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selScripting, "SelScripting", SELSCRIPTING_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selScripting);

		/* Register methods to main state */
	selLua->libFuncs(NULL, "Selene", seleneLib);
	selLua->libAddFuncs(NULL, "Selene", seleneExtLib);	/* and extended methods as well */

	return true;
}
