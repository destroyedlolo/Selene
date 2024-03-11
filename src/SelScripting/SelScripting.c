/* SelScripting.c
 *
 * Expose Selene's Lua scripting functions.
 *
 * 08/02/2024 First version
 */
#include <Selene/SelScripting.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>
#include <Selene/SelTimer.h>

#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <string.h>

static struct SelScripting selScripting;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;
static struct SelTimer *selTimer;

	/* ***
	 * Dependancies management
	 * ***/
static bool scc_checkdependencies(){	/* Ensure all dependancies are met */
	return(!!selTimer);
}

static bool scc_laterebuilddependancies(){	/* Add missing dependancies */
	selTimer = (struct SelTimer *)selCore->findModuleByName("SelTimer", SELTIMER_VERSION,'E');
	if(!selTimer){	/* We can live w/o it */
		selLog->Log('D', "SelTimer missing for SelScripting");
		return false;
	}

	return true;
}

	/* ***
	 * Methods exposed to main threads only
	 * ***/
static int ssl_Use(lua_State *L){
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
	struct SelModule *m = selCore->findModuleByName(name, 0, 0);

	if(m)	/* Already found */
		return 1;

	m = selCore->loadModule(name, 0, &verfound, 'E');	/* load it */

	if(m){
		if(m->initLua)
			m->initLua(selLua);
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
	if(sigfunc != LUA_REFNIL)
		selLua->pushtask(sigfunc, true);
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

static int ssl_WaitFor(lua_State *L){
/** 
 * @brief Wait for even to come or a task is scheduled.
 *
 *	The process is put on hold and doesn't consume any processor resources until waked up.
 *  Have a look on *Selenites* examples directory : this function is the **most important one**
 * to achieve resources conservatives automation with Séléné.
 * But take also in account 
 *  - your tasks will be executed ONLY if there is a WaitFor() loop
 *  - It's not multitasking at all. Consequently, tasks are expected to be
 *  as fast as possible and definitively not blocking.
 *
 * @function WaitFor
 * @param ... list of **SelTimer**, **SelEvent**, file IO.
 * @return number of events to proceed or 0 in case of error
 */
	unsigned int nsup=0;	/* Number of supervised object (used as index in the table) */
	int nre;				/* Number of received event */
	struct pollfd ufds[WAITMAXFD];
	int maxarg = lua_gettop(L);
	int i,j;

	for(j=1; j <= lua_gettop(L); j++){	/* Stacks SelTimer arguments */
		if(nsup == WAITMAXFD){
			selLog->Log('E', "Exhausting number of waiting FD, please increase WAITMAXFD");
			return 0;
		}

		void *r;
		if(( r = luaL_testudata(L, j, LUA_FILEHANDLE))){	/* We got a file */
			ufds[nsup].fd = fileno(*((FILE **)r));
			ufds[nsup++].events = POLLIN;
		} else if((r = luaL_testudata(L, j, "SelTimer"))){	/* We got a SelTimer */
			if(!selTimer){
				selLog->Log('E', "SelTimer module is not loaded");
				return 0;
			} else {
				ufds[nsup].fd = selTimer->getFD(r);
				ufds[nsup++].events = POLLIN;
			}
		} else {
			selLog->Log('E', "Unsupported type for WaitFor()");
			return 0;
		}
	}

		/* at least, we have to supervise todo list */
	if(nsup == WAITMAXFD){
		selLog->Log('E', "Exhausting number of waiting FD, please increase WAITMAXFD");
		return 0;
	}

	ufds[nsup].fd = selLua->getToDoListFD();	/* Push todo list's fd */
	ufds[nsup].events = POLLIN;
	nsup++;

		/* Waiting for events */
	if((nre = poll(ufds, nsup, -1)) == -1){ /* Let's consider it as not fatal */
		selLog->Log('E', strerror(errno));
		return 0;
	}

	for(i=0; i<nsup; i++){
		if( ufds[i].revents ){	/* This one has data */
			if(ufds[i].fd == selLua->getToDoListFD()){ /* Todo list's evenfd */
				uint64_t v;
				if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
					selLog->Log('E', "read(eventfd) : %s", strerror(errno));
				lua_pushcfunction(L, selLua->handleToDoList);	/*  Push the function to handle the todo list */
			}
		}
	}

	return lua_gettop(L)-maxarg;	/* Number of stuffs to proceed */
}

static const struct luaL_Reg seleneExtLib[] = {	/* Extended ones */
	{"WaitFor", ssl_WaitFor},
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

static int ssl_RegisterFunction(lua_State *L){
	return selLua->registerfunc(L);
}

static int ssl_TaskOnceConst(lua_State *L){
	return selLua->TaskOnceConst(L);
}

static int ssl_PushTaskByRef(lua_State *L){
	return selLua->PushTaskByRef(L);
}

static int ssl_PushTask(lua_State *L){
	return selLua->PushTask(L);
}

static int ssl_dumpToDoList(lua_State *L){
	return selLua->dumpToDoList(L);
}

static int ssl_LetsGo(lua_State *L){
/** 
 * @brief Do all late operation before running our application
 *
 * @function LetsGo
 */
	for(struct SelModule *m = modules; m; m=m->next){	/* Ensure all dependancies are met */
		if(!m->checkdependencies()){
			if(m->laterebuilddependancies)
				m->laterebuilddependancies();
		}
	}
	return 0;
}

static const struct luaL_Reg seleneLib[] = {
	{"Sleep", ssl_Sleep},
	{"Hostname", ssl_Hostname},
	{"getHostname", ssl_Hostname},
	{"getPid", ssl_getPID},
	{"RegisterFunction", ssl_RegisterFunction},
	{"TaskOnceConst", ssl_TaskOnceConst},
	{"PushTaskByRef", ssl_PushTaskByRef},
	{"PushTask", ssl_PushTask},
	{"dumpToDoList", ssl_dumpToDoList},
	{"LetsGo", ssl_LetsGo},
	{NULL, NULL} /* End of definition */
};

static void registerSelene(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "Selene", seleneLib);
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
	selLua = (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!selLua)
		return false;

		/* Other mandatory modules */

		/* optional modules */
	selTimer = NULL;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selScripting, "SelScripting", SELSCRIPTING_VERSION, LIBSELENE_VERSION))
		return false;

	selScripting.module.checkdependencies = scc_checkdependencies;
	selScripting.module.laterebuilddependancies = scc_laterebuilddependancies;

	registerModule((struct SelModule *)&selScripting);

		/* Register methods to main state
		 * Can't be called using selLog.module.initLua as not loaded by
		 * Selene.Use()
		 */
	registerSelene(NULL);
	selLua->libCreateOrAddFuncs(NULL, "Selene", seleneExtLib);	/* and extended methods as well */

	selLua->AddStartupFunc(registerSelene);
	return true;
}
