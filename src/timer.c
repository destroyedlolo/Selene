/* Timer.c
 *
 * This file contains timers stuffs
 *
 * 28/26/2015 LF : First version
 */

#include "selene.h"

#define __USE_POSIX199309	/* Otherwise some defines/types are not defined with -std=c99 */
#include <sys/timerfd.h>
#include <assert.h>

static const struct ConstTranscode _ClockMode[] = {
	{ "CLOCK_REALTIME", CLOCK_REALTIME },
	{ "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
	{ NULL, 0 }
};

static int ClockModeConst( lua_State *L ){
	return findConst(L, _ClockMode);
}

static int createtimer( lua_State *L ){
}

static int TimerRelease( lua_State *L ){
	return 0;
}

static const struct luaL_reg SelTimerLib [] = {
	{"create", createtimer},
	{"ClockModeConst", ClockModeConst},
	{NULL, NULL}
};

static const struct luaL_reg SelTimerM [] = {
	{"Release", TimerRelease},
	{"destroy", TimerRelease},	/* Alias */
	{NULL, NULL}
};

void _include_SelTimer( lua_State *L ){
	luaL_newmetatable(L, "SelTimer");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelTimerM);
	luaL_register(L,"SelTimer", SelTimerLib);
}
