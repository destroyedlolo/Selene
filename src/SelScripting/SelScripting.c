/* SelScripting.c
 *
 * Expose Selene's Lua scripting functions.
 *
 * SelScripting module provides full APIs and targets application based on
 * Séléné, where Séléné acts as a core component and manages all the aspects.
 *
 * SelLua provides reduced APIs to Séléné's based applications that
 * use Lua as helper as well as low level stuffs.
 *
 * 08/02/2024 First version
 */
#include <Selene/SelScripting.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>
#include <Selene/SelTimer.h>
#include <Selene/SelEvent.h>
#include <Selene/SelError.h>

#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static struct SelScripting ss_selScripting;

static struct SeleneCore *ss_selCore;
static struct SelLog *ss_selLog;
static struct SelLua *ss_selLua;
static struct SelTimer *ss_selTimer;
static struct SelEvent *ss_selEvent;
static struct SelError *ss_selError;

	/* ***
	 * Dependancies management
	 * ***/
static bool scc_checkdependencies(){	/* Ensure all dependencies are met */
	return(!!ss_selTimer && !!ss_selEvent);
}

static bool scc_laterebuilddependancies(){	/* Add missing dependencies */
	ss_selTimer = (struct SelTimer *)ss_selCore->findModuleByName("SelTimer", SELTIMER_VERSION, 0);
	if(!ss_selTimer){	/* We can live w/o it */
		ss_selLog->Log('D', "SelTimer missing for SelScripting");
	}

	ss_selEvent = (struct SelEvent *)ss_selCore->findModuleByName("SelEvent", SELEVENT_VERSION, 0);
	if(!ss_selEvent){	/* We can live w/o it */
		ss_selLog->Log('D', "SelEvent missing for SelScripting");
	}

	return true;
}

	/* ***
	 * Methods exposed to main threads only
	 * ***/
	
	/*
	 * Signal handling
	 */

static int sigfunc = LUA_REFNIL;	/* Function to call in case of SIG_INT */

static void sighandler(){
	if(sigfunc != LUA_REFNIL)
		ss_selLua->pushtask(sigfunc, true);
}

static int ssl_SigIntTask(lua_State *L){
/** 
 * Set **SIGINT** and **SIGUSR1** signal handler
 *
 * @function SigIntTask
 * @tparam function Function to be scheduled when a signal is received
 */
	if( lua_type(L, -1) == LUA_TFUNCTION ){
		sigfunc = ss_selLua->findFuncRef(L,lua_gettop(L));

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
 * @return number of events to proceed, a SelError in case of error
 */
	unsigned int nsup=0;	/* Number of supervised object (used as index in the table) */
	int nre;				/* Number of received event */
	struct pollfd ufds[WAITMAXFD];
	int maxarg = lua_gettop(L);
	int i,j;

	for(j=1; j <= lua_gettop(L); j++){	/* Stacks SelTimer arguments */
		if(nsup == WAITMAXFD){
			ss_selError->create(L, 'E', "Exhausting number of waiting FD, please increase WAITMAXFD", true);
			return 1;
		}

		void *r;
		if(( r = ss_selLua->testudata(L, j, LUA_FILEHANDLE))){	/* We got a file */
			ufds[nsup].fd = fileno(*((FILE **)r));
			ufds[nsup++].events = POLLIN;
		} else if((r = ss_selLua->testudata(L, j, "SelTimer"))){	/* We got a SelTimer */
			if(!ss_selTimer){
				ss_selError->create(L, 'E', "SelTimer module is not loaded", true);
				return 1;
			} else {
				ufds[nsup].fd = ss_selTimer->getFD(r);
				ufds[nsup++].events = POLLIN;
			}
		} else if((r = ss_selLua->testudata(L, j, "SelEvent"))){	/* We got a SelTimer */
			if(!ss_selEvent){
				ss_selError->create(L, 'E', "SelEvent module is not loaded", true);
				return 1;
			} else {
				ufds[nsup].fd = ss_selEvent->getFD(r);
				ufds[nsup++].events = POLLIN;
			}
		} else if(lua_type(L, j) == LUA_TNIL){
			ss_selLog->Log('E', "Argument #%d is unset", j);
			ss_selError->create(L, 'E', "Argument is unset", false);
			return 1;
		} else {
			ss_selLog->Log('E', "Argument #%d type '%s' is unsupported", j, lua_typename(L, lua_type(L, j)));
			ss_selError->create(L, 'E', "Unsupported type for WaitFor()", false);
			return 1;
		}
	}

		/* at least, we have to supervise todo list */
	if(nsup == WAITMAXFD){
		ss_selError->create(L, 'E', "Exhausting number of waiting FD, please increase WAITMAXFD", true);
		return 1;
	}

	ufds[nsup].fd = ss_selLua->getToDoListFD();	/* Push todo list's fd */
	ufds[nsup].events = POLLIN;
	nsup++;

		/* Waiting for events */
	if((nre = poll(ufds, nsup, -1)) == -1){ /* Let's consider it as not fatal */
		ss_selError->create(L, 'E', strerror(errno), true);
		return 1;
	}

	for(i=0; i<nsup; i++){
		if( ufds[i].revents ){	/* This one has data */
			if(ufds[i].fd == ss_selLua->getToDoListFD()){ /* Todo list's evenfd */
				uint64_t v;
				if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
					ss_selLog->Log('E', "read(eventfd) : %s", strerror(errno));
				lua_pushcfunction(L, ss_selLua->handleToDoList);	/*  Push the function to handle the todo list */
			} else for(j=1; j <= maxarg; j++){
					/* Note : no need to check for module availability as it
					 * has been done which checking the arguments
					 */
				void *r;
				if((r=ss_selLua->testudata(L, j, "SelTimer"))){
					if(ufds[i].fd == ss_selTimer->getFD(r) && !ss_selTimer->isDisabled(r)){
						uint64_t v;
						if(read( ufds[i].fd, &v, sizeof(uint64_t)) != sizeof(uint64_t))
							ss_selLog->Log('E', "read(timerfd) : %s", strerror(errno));
						if(ss_selTimer->getiFunc(r) != LUA_REFNIL){	/* Immediate function to be executed */
							lua_rawgeti(L, LUA_REGISTRYINDEX, ss_selTimer->getiFunc(r));
							if(lua_pcall(L, 0, 0, 0)){	/* Call the trigger without arg */
								ss_selLog->Log('E', "(SelTimer ifunc) %s", lua_tostring(L, -1));
								lua_pop(L, 1); /* pop error message from the stack */
								lua_pop(L, 1); /* pop NIL from the stack */
							}
						}
						if(ss_selTimer->getTask(r) != LUA_REFNIL){	/* Function to be pushed in todo list */
							if(ss_selLua->pushtask(ss_selTimer->getTask(r), ss_selTimer->getOnce(r))){
								ss_selLog->Log('F', "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
								lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
								lua_error(L);
								exit(EXIT_FAILURE);	/* Code never reached */
							}
						}
					}
				} else if((r=ss_selLua->testudata(L, j, "SelEvent"))){
					if(ufds[i].fd == ss_selEvent->getFD(r)){
						if(ss_selLua->pushtask(ss_selEvent->getFunc(r), false) ){
							ss_selLog->Log('F', "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
							lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
							lua_error(L);
							exit(EXIT_FAILURE);	/* Code never reached */
						}
					}
				} else if(( r = ss_selLua->testudata(L, j, LUA_FILEHANDLE))){
					if(ufds[i].fd == fileno(*((FILE **)r)))
						lua_pushvalue(L, j);
				}
			}
		}
	}

	return lua_gettop(L)-maxarg;	/* Number of stuffs to proceed */
}

static const struct luaL_Reg seleneExtLib[] = {	/* Extended ones */
	{"WaitFor", ssl_WaitFor},
	{"SigIntTask", ssl_SigIntTask},
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

static int ssl_RegisterFunction(lua_State *L){
	return ss_selLua->registerfunc(L);
}

static int ssl_TaskOnceConst(lua_State *L){
	return ss_selLua->TaskOnceConst(L);
}

static int ssl_PushTaskByRef(lua_State *L){
	return ss_selLua->PushTaskByRef(L);
}

static int ssl_PushTask(lua_State *L){
	return ss_selLua->PushTask(L);
}

static int ssl_HWTask(lua_State *L){
	lua_pushboolean(L, !ss_selLua->isToDoListEmpty());
	return 1;
}

static int ssl_dumpToDoList(lua_State *L){
	return ss_selLua->dumpToDoList(L);
}

static const struct luaL_Reg seleneLib[] = {
	{"Sleep", ssl_Sleep},
	{"RegisterFunction", ssl_RegisterFunction},
	{"TaskOnceConst", ssl_TaskOnceConst},
	{"PushTaskByRef", ssl_PushTaskByRef},
	{"PushTask", ssl_PushTask},
	{"HasWaitingTask", ssl_HWTask},
	{"dumpToDoList", ssl_dumpToDoList},
	{NULL, NULL} /* End of definition */
};

static void registerSelene(lua_State *L){
	ss_selLua->libCreateOrAddFuncs(L, "Selene", seleneLib);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
	ss_selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!ss_selCore)
		return false;
	ss_selLog = (struct SelLog *)ss_selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!ss_selLog)
		return false;
	ss_selLua = (struct SelLua *)ss_selCore->findModuleByName("SelLua", SELLUA_VERSION,'F');
	if(!ss_selLua)
		return false;

		/* Other mandatory modules */
	uint16_t found;
	ss_selError = (struct SelError *)ss_selCore->loadModule("SelError", SELERROR_VERSION, &found, 'F');
	if(!ss_selError)
		return false;

		/* optional modules */
	ss_selTimer = NULL;
	ss_selEvent = NULL;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&ss_selScripting, "SelScripting", SELSCRIPTING_VERSION, LIBSELENE_VERSION))
		return false;

	ss_selScripting.module.checkdependencies = scc_checkdependencies;
	ss_selScripting.module.laterebuilddependancies = scc_laterebuilddependancies;

	registerModule((struct SelModule *)&ss_selScripting);

		/* Register methods to main state
		 * Can't be called using ss_selLog.module.initLua as not loaded by
		 * Selene.Use()
		 */
	ss_selLua->exposeAdminAPI(ss_selLua->getLuaState());

	registerSelene(NULL);
	ss_selLua->libCreateOrAddFuncs(NULL, "Selene", seleneExtLib);	/* and extended methods as well */

	ss_selLua->AddStartupFunc(registerSelene);
	return true;
}
