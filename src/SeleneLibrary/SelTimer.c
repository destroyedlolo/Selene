/***
Multi purposes and versatile timer.

@classmod SelTimer

 * History :
 * 28/06/2015 LF : First version
 * 03/07/2015 LF : Argument of Timer:create() is now an array
 * 20/01/2016 LF : rename as SelTimer.c
 * 12/05/2016 LF : Add "at" argument
 * 19/05/2016 LF : Bypass a rounding bug in GCC
 *
 * 05/04/2018 LF : Switch to Selene v4
 */

#include "libSelene.h"
#include "SelTimer.h"

#include <sys/timerfd.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

static const struct ConstTranscode _ClockMode[] = {
	{ "CLOCK_REALTIME", CLOCK_REALTIME },
	{ "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
	{ NULL, 0 }
};

static int ClockModeConst( lua_State *L ){
	return findConst(L, _ClockMode);
}

static struct SelTimer *checkSelTimer(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelTimer");
	luaL_argcheck(L, r != NULL, 1, "'SelTimer' expected");
	return (struct SelTimer *)r;
}

static int TimerCreate(lua_State *L){
/** 
 * Create a new SelAverageCollection.
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
 * @field when initial delay (seconds)
 * @field at initial launch (format HH.MM -> *10.45* for *10:45*)
 * @field interval delay b/w next run (seconds)
 * @field clockid clock mode *CLOCK\_REALTIME* (default) or *CLOCK\_MONOTONIC* (see Linux documentation) [@{Create} only]
 * @field ifunc function to run "immediately" when a timer is over
 * @field task function to put in task list
 * @field once avoid task duplication (don't push it again if already in the todo list)
 */
	struct SelTimer *timer;
	int clockid = CLOCK_REALTIME, t;
	lua_Number awhen = 0, arep = 0;
	int ifunc = LUA_REFNIL;
	int task = LUA_REFNIL;
	int task_once = -1;
	struct itimerspec itval;
	bool set = false;

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "Timer.create() is expecting a table");
		return 2;
	}

	lua_pushstring(L, "when");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER ){
		awhen = lua_tonumber(L, -1);
		set = true;
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "at");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER ){
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
	if( lua_type(L, -1) == LUA_TNUMBER )
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
		ifunc = findFuncRef(L,lua_gettop(L));
	lua_pop(L,1);
	
	lua_pushstring(L, "task");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TFUNCTION )
		task = findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
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

	timer = (struct SelTimer *)lua_newuserdata(L, sizeof( struct SelTimer ));
	luaL_getmetatable(L, "SelTimer");
	lua_setmetatable(L, -2);
	timer->fd = t;
	timer->ifunc = ifunc;
	timer->task = task;
	timer->once = task_once;
	timer->disable = 0;
	timer->when = awhen;
	timer->rep = arep;

	if(set && timerfd_settime( timer->fd, 0, &itval, NULL ) == -1 ){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	return 1;
}

static int TimerRelease( lua_State *L ){
/** 
 * Release all resources associated with the timer, making it unusable.
 *
 * @function Release
 */

	struct SelTimer *timer = checkSelTimer(L);

	if(timer->fd == -1){
		lua_pushnil(L);
		lua_pushstring(L, "Release() on a dead object");
		return 2;
	}

	close(timer->fd);
	timer->fd = -1;

	return 0;
}

static int TimerSet( lua_State *L ){
/** 
 * Update the timer.
 *
 * if both **at** and **when** are present, the last one is took in account.
 *
 * @function Set
 * @tparam table create_arguments
 * @see create_arguments
 */

	struct SelTimer *timer = checkSelTimer(L);
	struct itimerspec itval;

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "Timer.Set() is expecting a table");
		return 2;
	}

		/* Get the current value */
	if( timerfd_gettime( timer->fd, &itval ) == -1 ){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

		/* Check if values have to be changed */
	lua_pushstring(L, "when");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER ){
		timer->when = lua_tonumber(L, -1);
		itval.it_value.tv_sec = (time_t)timer->when;
		itval.it_value.tv_nsec = (unsigned long int)((timer->when - (time_t)timer->when) * 1e9);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "at");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER ){
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
	if( lua_type(L, -1) == LUA_TNUMBER ){
		timer->rep = lua_tonumber(L, -1);
		itval.it_interval.tv_sec = (time_t)timer->rep;
		itval.it_interval.tv_nsec = (unsigned long int)((timer->rep - (time_t)timer->rep) * 1e9);
	}
	lua_pop(L, 1);	/* cleaning ... */

	lua_pushstring(L, "ifunc");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TFUNCTION )
		timer->ifunc = findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
	lua_pop(L,1);
	
	lua_pushstring(L, "task");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TFUNCTION )
		timer->task = findFuncRef(L,lua_gettop(L));	/* and the function is part of the main context */
	lua_pop(L,1);

	if( timerfd_settime( timer->fd, 0, &itval, NULL ) == -1 ){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
		
	return 0;
}

const char *_TimerReset( struct SelTimer *timer ){	/* Used also to clear watchdogs */
	struct itimerspec itval;

	if(timer->fd == -1)
		return("Reset() on a dead object");

	itval.it_value.tv_sec = (time_t)timer->when;
	itval.it_value.tv_nsec = (unsigned long int)((timer->when - (time_t)timer->when) * 1e9);
	itval.it_interval.tv_sec = (time_t)timer->rep;
	itval.it_interval.tv_nsec = (unsigned long int)((timer->rep - (time_t)timer->rep) * 1e9);

	if( timerfd_settime( timer->fd, 0, &itval, NULL ) == -1 )
		return(strerror(errno));

	return NULL;
}

static int TimerReset( lua_State *L ){
/** 
 * Reset the timer
 *
 * @function Reset
 */
	struct SelTimer *timer = checkSelTimer(L);
	const char *err=_TimerReset( timer );

	if(err){
		lua_pushnil(L);
		lua_pushstring(L, err);
	}

	return 0;
}

static int TimerDisable( lua_State *L ){
/** 
 * Disable the timer, no function will be launched or scheduled
 *
 * @function Disable
 */
	struct SelTimer *timer = checkSelTimer(L);

	timer->disable = -1;

	return 0;
}

static int TimerEnable( lua_State *L ){
/** 
 * Enable the timer.
 *
 * Notez-bien : no function is launched or scheduled if the timer is already exhausted.
 *
 * @function Enable
 */
	struct SelTimer *timer = checkSelTimer(L);

	timer->disable = 0;

	return 0;
}

static int TimerGet( lua_State *L ){
/** 
 * Get the duration to the next trigger (in seconds)
 *
 * @function Get
 * @treturn num remaining seconds
 */
	struct SelTimer *timer = checkSelTimer(L);
	struct itimerspec itval;
	lua_Number cnt;

	if(timer->fd == -1){
		lua_pushnil(L);
		lua_pushstring(L, "Get() on a dead object");
		return 2;
	}

	if( timerfd_gettime( timer->fd, &itval ) == -1 ){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	cnt = itval.it_value.tv_sec + (lua_Number)itval.it_value.tv_nsec / 1e9;
	lua_pushnumber(L, cnt);

	return 1;
}

static const struct luaL_Reg SelTimerLib [] = {
	{"Create", TimerCreate},
#ifdef COMPATIBILITY
	{"create", smq_connect},
#endif
	{"ClockModeConst", ClockModeConst},
	{NULL, NULL}
};

static const struct luaL_Reg SelTimerM [] = {
	{"Set", TimerSet},
	{"Get", TimerGet},
	{"Reset", TimerReset},
	{"Release", TimerRelease},
	{"destroy", TimerRelease},	/* Alias */
	{"Disable", TimerDisable},
	{"Enable", TimerEnable},	/* Alias */
	{NULL, NULL}
};

int initSelTimer( lua_State *L ){
	libSel_objFuncs( L, "SelTimer", SelTimerM);
	libSel_libFuncs( L, "SelTimer", SelTimerLib );

	return 1;
}
