/* Timer.c
 *
 * This file contains timers stuffs
 *
 * 28/06/2015 LF : First version
 * 03/07/2015 LF : Argument of Timer:create() is now an array
 */

#define __USE_POSIX199309	/* Otherwise some defines/types are not defined with -std=c99 */
#define	_POSIX_C_SOURCE 199309L

#include "selene.h"

#include <sys/timerfd.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "Timer.h"

static const struct ConstTranscode _ClockMode[] = {
	{ "CLOCK_REALTIME", CLOCK_REALTIME },
	{ "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
	{ NULL, 0 }
};

static int ClockModeConst( lua_State *L ){
	return findConst(L, _ClockMode);
}

static struct SelTimer *checkSelTimer(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelTimer");
	luaL_argcheck(L, r != NULL, 1, "'SelTimer' expected");
	return (struct SelTimer *)r;
}

static int TimerCreate(lua_State *L){
/* Create a timer
 * -> 1: initial delay (seconds)
 *    2: interval (seconds)
 *    3: (optional), clock mode
 * <- the new trigger
 */
	struct SelTimer *timer;
	int clockid = CLOCK_REALTIME, t;
	lua_Number awhen = 0, arep = 0;
	int ifunc = LUA_REFNIL;
	int task = LUA_REFNIL;
	int task_once = -1;
	struct itimerspec itval;

	if(!lua_istable(L, -1)){	/* Argument has to be a table */
		lua_pushnil(L);
		lua_pushstring(L, "Timer.create() is expecting a table");
		return 2;
	}

	lua_pushstring(L, "when");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TNUMBER ){
		awhen = lua_tonumber(L, -1);
		lua_pop(L, 1);	/* cleaning ... */
	} else {
		lua_pop(L, 1);	/* cleaning ... */
		lua_pushnil(L);
		lua_pushstring(L, "Timer.create() is expecting a numeric and non null \"when\" argument");
		return 2;
	}

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
	if( lua_type(L, -1) != LUA_TFUNCTION )	/* This function is optional */
		lua_pop(L, 1);	/* Pop the unused result */
	else
		ifunc = luaL_ref(L,LUA_REGISTRYINDEX);	/* and the function is part of the main context */
	
	lua_pushstring(L, "task");
	lua_gettable(L, -2);
	if( lua_type(L, -1) != LUA_TFUNCTION )	/* This function is optional */
		lua_pop(L, 1);	/* Pop the unused result */
	else
		task = luaL_ref(L,LUA_REGISTRYINDEX);	/* and the function is part of the main context */

	lua_pushstring(L, "once");
	lua_gettable(L, -2);
	if( lua_type(L, -1) == LUA_TBOOLEAN )
		task_once = lua_toboolean(L, -1);
	lua_pop(L, 1);	/* Pop the value */

#if 0
		/* Well, potentially a callback less timer can be created if the 
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


	if( timerfd_settime( timer->fd, 0, &itval, NULL ) == -1 ){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	return 1;
}

static int TimerRelease( lua_State *L ){
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
	return 0;
}

static int TimerGet( lua_State *L ){
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

static const struct luaL_reg SelTimerLib [] = {
	{"create", TimerCreate},
	{"ClockModeConst", ClockModeConst},
	{NULL, NULL}
};

static const struct luaL_reg SelTimerM [] = {
	{"Set", TimerSet},
	{"Get", TimerGet},
	{"Release", TimerRelease},
	{"destroy", TimerRelease},	/* Alias */
	{NULL, NULL}
};

void init_SelTimer( lua_State *L ){
	luaL_newmetatable(L, "SelTimer");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelTimerM);
	luaL_register(L,"SelTimer", SelTimerLib);
}
