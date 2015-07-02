/* Timer.c
 *
 * This file contains timers stuffs
 *
 * 28/26/2015 LF : First version
 */

#include "selene.h"

#define __USE_POSIX199309	/* Otherwise some defines/types are not defined with -std=c99 */
#include <sys/timerfd.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

static const struct ConstTranscode _ClockMode[] = {
	{ "CLOCK_REALTIME", CLOCK_REALTIME },
	{ "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
	{ NULL, 0 }
};

static int ClockModeConst( lua_State *L ){
	return findConst(L, _ClockMode);
}


static int TimerCreate( lua_State *L ){
/* Create a timer
 * -> 1: initial delay (seconds)
 *    2: interval (seconds)
 *    3: (optional), clock mode
 * <- the new trigger
 */
	int *timer, t;
	int clockid = CLOCK_REALTIME;
	struct itimerspec itval;

	lua_Number awhen = luaL_checknumber(L, 1);
	lua_Number arep = luaL_checknumber(L, 2);

	if( lua_gettop(L) > 2 ){	/* Clockid provided */
		clockid = luaL_checkint(L, 3);
		lua_pop(L, 1); /* pop the clockid */
	}
	lua_pop(L, 2);	/* pop the initial delay */

printf("(%d) n: %lf\n", lua_gettop(L), awhen);
	itval.it_value.tv_sec = (time_t)awhen;
	itval.it_value.tv_nsec = (unsigned long int)((awhen - (time_t)awhen) * 1e9);
	itval.it_interval.tv_sec = (time_t)arep;
	itval.it_interval.tv_nsec = (unsigned long int)((arep - (time_t)arep) * 1e9);

printf("%ld . %ld, %ld . %ld\n", 
	itval.it_value.tv_sec, itval.it_value.tv_nsec,
	itval.it_interval.tv_sec, itval.it_value.tv_nsec
);

	if((t = timerfd_create( clockid, 0 )) == -1){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	timer = (int *)lua_newuserdata(L, sizeof( int ));
	luaL_getmetatable(L, "SelTimer");
	lua_setmetatable(L, -2);

	return 1;
}

static int TimerRelease( lua_State *L ){
	return 0;
}

static int TimerSet( lua_State *L ){
	return 0;
}

static const struct luaL_reg SelTimerLib [] = {
	{"create", TimerCreate},
	{"ClockModeConst", ClockModeConst},
	{NULL, NULL}
};

static const struct luaL_reg SelTimerM [] = {
	{"set", TimerSet},
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
