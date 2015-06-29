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

static const struct ConstTranscode _ClockMode[] = {
	{ "CLOCK_REALTIME", CLOCK_REALTIME },
	{ "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
	{ NULL, 0 }
};

static int ClockModeConst( lua_State *L ){
	return findConst(L, _ClockMode);
}


static int everytimer( lua_State *L ){	/* Repeating timer */
	int *timer;
	int clockid = CLOCK_REALTIME;
	struct itimerspec itval;
	double ip, fp;
	lua_Number aevery = luaL_checknumber(L, 1);
	if( lua_gettop(L) > 1 ){	/* Clockid provided */
		clockid = luaL_checkint(L, 2);
		lua_pop(L, 1); /* pop the clockid */
	}
	lua_pop(L, 1);	/* pop the interval */

printf("n: %lf\n", aevery);
	fp = modf( aevery, &ip );
	itval.it_value.tv_sec = (time_t)ip;
	itval.it_value.tv_nsec = (long int)(10e+9 * fp);
printf("%ld . %ld\n", itval.it_value.tv_sec, itval.it_value.tv_nsec);

	timer = (int *)lua_newuserdata(L, sizeof( int ));
	return 0;
}

static int TimerRelease( lua_State *L ){
	return 0;
}

static const struct luaL_reg SelTimerLib [] = {
	{"every", everytimer},
	{"ClockModeConst", ClockModeConst},
	{NULL, NULL}
};

static const struct luaL_reg SelTimerM [] = {
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
