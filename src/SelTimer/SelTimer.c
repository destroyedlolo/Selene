/* SelTimer.h
 *
 * Multi purposes and versatile timer
 *
 * Notez-bien : No C interface as we're using C native functionalities and
 * callbacks are useful only at Lua side.
 *
 * 09/03/2024 First version
 */

#include <Selene/SelTimer.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>
#include <Selene/SelScripting.h>

#include "selTimerStorage.h"

#include <sys/timerfd.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

struct SelTimer selTimer;

struct SeleneCore *selCore;
struct SelLog *selLog;
struct SelLua *selLua;
static struct SelScripting *selScripting;

static struct selTimerStorage *checkSelTimer(lua_State *L){
	struct selTimerStorage **r = selLua->testudata(L, 1, "SelTimer");
	luaL_argcheck(L, r != NULL, 1, "'SelTimer' expected");
	return *r;
}

static const struct ConstTranscode _ClockMode[] = {
	{ "CLOCK_REALTIME", CLOCK_REALTIME },
	{ "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
	{ NULL, 0 }
};

static int stl_ClockModeConst( lua_State *L ){
	return selLua->findConst(L, _ClockMode);
}

static struct selTimerStorage *stc_find(const char *name, unsigned int h){
/** 
 * Find a SelTimer by its name.
 *
 * @function Find
 * @tparam string name Name of the timer
 * @param int hash code (recomputed if null)
 * @treturn ?SelAverageCollection|nil
 */
	return((struct selTimerStorage *)selCore->findNamedObject((struct SelModule *)&selTimer, name, h));
}

static int stl_find(lua_State *L){
	struct selTimerStorage *col = selTimer.find(luaL_checkstring(L, 1), 0);
	if(!col)
		return 0;

	struct selTimerStorage **r = lua_newuserdata(L, sizeof(struct selTimerStorage *));
	assert(r);
	luaL_getmetatable(L, "SelTimer");
	lua_setmetatable(L, -2);
	*r = col;

	return 1;
}

static int stl_TimerCreate(lua_State *L){
/** 
 * @brief Create a new SelTimer
 *
 * if both **at** and **when** are present, the last one is took in account.
 *
 * @function Create
 * @tparam table create_arguments
 * @see create_arguments
 * @usage
 timer = SelTimer.Create { when=3.5, interval=1, clockid=SelTimer.ClockModeConst("CLOCK_MONOTONIC") }
 */
/**
 * Arguments for @{Create} and @{Set}
 *
 * @table create_arguments
 * @filed Name Name of this timer (mandatory)
 * @field when initial delay (seconds)
 * @field at initial launch (format HH.MM -> *10.45* for *10:45*)
 * @field interval delay b/w next run (seconds)
 * @field clockid clock mode *CLOCK\_REALTIME* (default) or *CLOCK\_MONOTONIC* (see Linux documentation) [@{Create} only]
 * @field ifunc function to run "immediately" when a timer is over
 * @field task function to put in task list
 * @field once avoid task duplication (don't push it again if already in the todo list)
 */
	struct selTimerStorage *timer;
	int clockid = CLOCK_REALTIME, t;
	lua_Number awhen = 0, arep = 0;
	int ifunc = LUA_REFNIL;
	int task = LUA_REFNIL;
	int task_once = true;
	struct itimerspec itval;
	bool set = false;
	const char *name = NULL;

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "Timer.Create() is expecting a table");
		return 2;
	}

	lua_pushstring(L, "Name");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TSTRING){
		name = lua_tostring(L, -1);
		unsigned int h = selL_hash(name);
		if(stc_find(name, h))
			return luaL_error(L, "This SelTimer already exists");
	} else
		selLog->Log('D', "Unamed timer used");
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "when");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		awhen = lua_tonumber(L, -1);
		set = true;
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "at");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		time_t now, when;
		struct tm tmt;
		int h,m;

		time(&now);
		localtime_r( &now, &tmt );

		m = ((int)nearbyint((awhen = lua_tonumber(L, -1)) * 100)) % 100;
		h = (int)awhen;

		if( tmt.tm_hour * 60 + tmt.tm_min > h * 60 + m )
			h += 24;	/* If the requested time is in the past
					 * we switch to next day
					 */
		tmt.tm_hour = h;
		tmt.tm_min = m;
		tmt.tm_sec = 0;
		when = mktime( &tmt );
		awhen = difftime( when, now );

		set = true;
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "interval");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER)
		arep = lua_tonumber(L, -1);
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "clockid");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER )
		clockid = lua_tointeger(L, -1);
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "ifunc");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TFUNCTION )
		ifunc = selScripting->findFuncRef(L,lua_gettop(L));
	lua_pop(L,1);
	
	lua_pushstring(L, "task");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TFUNCTION )
		task = selScripting->findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
	lua_pop(L,1);

	lua_pushstring(L, "once");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TBOOLEAN )
		task_once = lua_toboolean(L, -1);
	else if( lua_type(L, -1) == LUA_TNUMBER )
		task_once = lua_tointeger(L, -1);
	lua_pop(L, 1);	/* Pop the value */

#if 0
		/* Well, potentially a callbackless timer can be created if the 
		 * program is polling on Timer:Get() value.
		 * For the moment, comment it out, will see ...
		 */
	if(ifunc == LUA_REFNIL && task == LUA_REFNIL){
		lua_pushnil(L);
		lua_pushstring(L, "Defining a Timer without callback function is useless");
		return 2;
	}
#endif

	itval.it_value.tv_sec = (time_t)awhen;
	itval.it_value.tv_nsec = (unsigned long int)((awhen - (time_t)awhen) * 1e9);
	itval.it_interval.tv_sec = (time_t)arep;
	itval.it_interval.tv_nsec = (unsigned long int)((arep - (time_t)arep) * 1e9);

	if((t = timerfd_create( clockid, 0 )) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	timer = malloc(sizeof( struct selTimerStorage ));
	assert(timer);

	luaL_getmetatable(L, "SelTimer");
	lua_setmetatable(L, -2);
	timer->fd = t;
	timer->ifunc = ifunc;
	timer->task = task;
	timer->once = task_once;
	timer->disable = false;
	timer->when = awhen;
	timer->rep = arep;

	if(set && timerfd_settime( timer->fd, 0, &itval, NULL ) == -1 ){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

		/* Register this collection (only if named)*/
	if(name)
		selCore->registerNamedObject((struct SelModule *)&selTimer, (struct _SelNamedObject *)timer, strdup(name));
	else
		selCore->initObject((struct SelModule *)&selTimer, (struct SelObject *)timer);


		/* Create Lua Object */
	struct selTimerStorage **p = lua_newuserdata(L, sizeof( struct selTimerStorage *));
	assert(p);
	*p = timer;

	luaL_getmetatable(L, "SelTimer");
	lua_setmetatable(L, -2);

	MCHECK;
	return 1;
}

static const struct luaL_Reg SelTimerLib [] = {
	{"ClockModeConst", stl_ClockModeConst},
	{"Find", stl_find},
	{NULL, NULL}
};

static const struct luaL_Reg SelTimerMainLib [] = {
	{"Create", stl_TimerCreate},
	{NULL, NULL}
};

static int stl_TimerRelease(lua_State *L){
/** 
 * @brief Release all resources associated with the timer, making it unusable.
 *
 * @function Release
 */
	struct selTimerStorage *timer = checkSelTimer(L);

	if(timer->fd == -1){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	close(timer->fd);
	timer->fd = -1;

	return 0;
}

static int stl_TimerSet(lua_State *L){
/** 
 * @brief Update the timer.
 *
 * if both **at** and **when** are present, the last one is took in account.
 *
 * @function Set
 * @tparam table create_arguments
 * @see create_arguments
 */

	struct selTimerStorage *timer = checkSelTimer(L);
	struct itimerspec itval;

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "Timer.Set() is expecting a table");
		return 2;
	}

		/* Get the current value */
	if(timerfd_gettime( timer->fd, &itval ) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

		/* Check if values have to be changed */
	lua_pushstring(L, "when");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		timer->when = lua_tonumber(L, -1);
		itval.it_value.tv_sec = (time_t)timer->when;
		itval.it_value.tv_nsec = (unsigned long int)((timer->when - (time_t)timer->when) * 1e9);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "at");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		time_t now, when;
		struct tm tmt;
		int h,m;
		lua_Number awhen;

		time(&now);
		localtime_r( &now, &tmt );

		m = ((int)nearbyint((awhen = lua_tonumber(L, -1)) * 100)) % 100;
		h = (int)awhen;

		if( tmt.tm_hour * 60 + tmt.tm_min > h * 60 + m )
			h += 24;	/* If the requested time is in the past
					 * we switch to next day
					 */
		tmt.tm_hour = h;
		tmt.tm_min = m;
		tmt.tm_sec = 0;
		when = mktime( &tmt );
		awhen = difftime( when, now );
		itval.it_value.tv_sec = (time_t)awhen;
		itval.it_value.tv_nsec = (unsigned long int)((timer->when - (time_t)timer->when) * 1e9);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "interval");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TNUMBER){
		timer->rep = lua_tonumber(L, -1);
		itval.it_interval.tv_sec = (time_t)timer->rep;
		itval.it_interval.tv_nsec = (unsigned long int)((timer->rep - (time_t)timer->rep) * 1e9);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "ifunc");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TFUNCTION)
		timer->ifunc = selScripting->findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
	lua_pop(L,1);
	
	lua_pushstring(L, "task");
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TFUNCTION)
		timer->task = selScripting->findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
	lua_pop(L,1);

	if(timerfd_settime( timer->fd, 0, &itval, NULL ) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
		
	return 0;
}

static int stl_TimerGet(lua_State *L){
/** 
 * @brief Get the duration to the next trigger (in seconds)
 *
 * @function Get
 * @treturn num remaining seconds
 */
	struct selTimerStorage *timer = checkSelTimer(L);
	struct itimerspec itval;
	lua_Number cnt;

	if(timer->fd == -1){
		lua_pushnil(L);
		lua_pushstring(L, "Get() on a dead object");
		return 2;
	}

	if(timerfd_gettime( timer->fd, &itval ) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	cnt = itval.it_value.tv_sec + (lua_Number)itval.it_value.tv_nsec / 1e9;
	lua_pushnumber(L, cnt);

	return 1;
}

static const char *stc_reset(struct selTimerStorage *timer){
	struct itimerspec itval;

	if(timer->fd == -1)
		return("Reset() on a dead object");

	itval.it_value.tv_sec = (time_t)timer->when;
	itval.it_value.tv_nsec = (unsigned long int)((timer->when - (time_t)timer->when) * 1e9);
	itval.it_interval.tv_sec = (time_t)timer->rep;
	itval.it_interval.tv_nsec = (unsigned long int)((timer->rep - (time_t)timer->rep) * 1e9);

	if( timerfd_settime( timer->fd, 0, &itval, NULL ) == -1 ){
		const char *err = strerror(errno);
		selLog->Log('E', "timerfd_settime() : %s", err);
		return(err);
	}

	return NULL;
}

static int stl_TimerReset(lua_State *L){
/** 
 * @brief Reset the timer
 *
 * @function Reset
 */
	struct selTimerStorage *timer = checkSelTimer(L);
	const char *err = stc_reset(timer);

	if(err){
		lua_pushnil(L);
		lua_pushstring(L, err);
	}

	return 0;
}

static int stl_TimerDisable( lua_State *L ){
/** 
 * @brief Disable the timer, no function will be launched or scheduled
 *
 * @function Disable
 */
	struct selTimerStorage *timer = checkSelTimer(L);

	timer->disable = -1;

	return 0;
}

static int stl_TimerEnable( lua_State *L ){
/** 
 * @brief Enable the timer.
 *
 * Notez-bien : no function is launched or scheduled if the timer is already exhausted.
 *
 * @function Enable
 */
	struct selTimerStorage *timer = checkSelTimer(L);

	timer->disable = 0;

	return 0;
}

static const struct luaL_Reg SelTimerM [] = {
	{"Release", stl_TimerRelease},
	{"destroy", stl_TimerRelease},	/* Alias */
	{"Set", stl_TimerSet},
	{"Get", stl_TimerGet},
	{"Reset", stl_TimerReset},
	{"Disable", stl_TimerDisable},
	{"Enable", stl_TimerEnable},
	{NULL, NULL}
};

static void registerSelTimer(lua_State *L){
	selLua->libCreateOrAddFuncs(L, "SelTimer", SelTimerLib);
	selLua->objFuncs(L, "SelTimer", SelTimerM);
}

static int stc_getFD(void *r){
	return (*(struct selTimerStorage **)r)->fd;
}

static int stc_getiFunc(void *r){
	return (*(struct selTimerStorage **)r)->ifunc;
}

static int stc_getTask(void *r){
	return (*(struct selTimerStorage **)r)->task;
}

static bool stc_getOnce(void *r){
	return (*(struct selTimerStorage **)r)->once;
}

static bool stc_isDisabled(void *r){
	return (*(struct selTimerStorage **)r)->disable;
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
		/* Core modules */
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
	selScripting = (struct SelScripting *)selCore->findModuleByName("SelScripting", SELSCRIPTING_VERSION,'F');
	if(!selScripting)
		return false;

		/* optional modules */

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selTimer, "SelTimer", SELTIMER_VERSION, LIBSELENE_VERSION))
		return false;

	selTimer.reset = stc_reset;
	selTimer.getFD = stc_getFD;
	selTimer.getiFunc = stc_getiFunc;
	selTimer.getTask = stc_getTask;
	selTimer.getOnce = stc_getOnce;
	selTimer.isDisabled = stc_isDisabled;
	selTimer.find = stc_find;

	registerModule((struct SelModule *)&selTimer);

	selLua->libCreateOrAddFuncs(NULL, "SelTimer", SelTimerMainLib);
	registerSelTimer(NULL);
	selLua->AddStartupFunc(registerSelTimer);

	return true;
}
