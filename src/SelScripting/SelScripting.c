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

static struct SelScripting selScripting;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static int ssl_Sleep( lua_State *L ){
/** 
 * Sleep some seconds.
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
 * Get the host's name.
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
 * Get the current process ID
 *
 * @function getPid
 * @treturn num PID of the current process
 */
	lua_pushinteger(L, getpid());
	return 1;
}


	/* Methods exposed to slave threads as well */
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

		/* Register functions to main state */
	selLua->libFuncs(NULL, "Selene", seleneLib);

	return true;
}
