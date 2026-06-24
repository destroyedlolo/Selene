/***
 * LCD screen impersonation
 *
 * Most of the C functions are defined in at module's level
 * (in SelLCD.c)
 */

#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDScreen.h>
#include <Selene/SelPlug-in/SelLCD/SelLCDSubSurface.h>

#include <errno.h>	/* EBUSY */
#include <stdlib.h>	/* malloc() */
#include <assert.h>
#include <string.h>

struct SGS_callbacks cb_screen;

static struct SelLCDScreenLua *checkSelLCDScreen(lua_State *L){
	void *r = slcd_selLua->testudata(L, 1, "SelLCDScreen");
	luaL_argcheck(L, r != NULL, 1, "'SelLCDScreen' expected");

	return (struct SelLCDScreenLua *)r;
}

static int lcdl_Shutdown(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);

	slcd_selLCD.Shutdown(lcd->storage);

	return 0;
}

static int lcdl_Backlight(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	bool bl = lua_toboolean(L, 2);

	slcd_selLCD.Backlight(lcd->storage, bl);

	return 0;
}

static int lcdl_DisplayCtl(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	bool screen = lua_toboolean(L, 2);
	bool cursor = lua_toboolean(L, 3);
	bool blink = lua_toboolean(L, 4);

	slcd_selLCD.DisplayCtl(lcd->storage, screen, cursor, blink);

	return 0;
}

static int lcdl_EntryCtl(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	bool inc = lua_toboolean(L, 2);
	bool shift = lua_toboolean(L, 3);

	slcd_selLCD.EntryCtl(lcd->storage, inc, shift);

	return 0;
}

static int lcdl_SetTiming(lua_State *L){
/**
 * @brief Set LCD timming
 *
 *	It's an optimisation function. Default value are safe, yours are ...
 * on your hand only.
 *
 * @function SetTiming
 * @param E timing in microsecond
 * @param process timing in microsecond
 */
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	lcd->storage->clock_pulse = luaL_checkinteger(L, 2);
	lcd->storage->clock_process = luaL_checkinteger(L, 3);

#ifdef DEBUG
	slcd_selLog->Log('T', "SelLCD.SetTiming(%p, %ld, %ld)", lcd->storage, lcd->storage->clock_pulse, lcd->storage->clock_process);
#endif

	return 0;
}

static int lcdl_SetDDRAM(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	uint8_t pos = lua_toboolean(L, 2);

	slcd_selLCD.SetDDRAM(lcd->storage, pos);

	return 0;
}

static int lcdl_SetSize(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	uint32_t w = lua_tonumber(L, 2);
	uint32_t h = lua_tonumber(L, 3);

	slcd_selLCD.SetSize(lcd->storage, w,h);

	return 0;
}

static int lcdl_bClear(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	slcd_selLCD.bClear(lcd->storage);

	return 0;
}

static int lcdl_bWriteString(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	const char *s = luaL_checkstring(L, 2);

	slcd_selLCD.bWriteString(lcd->storage, s);

	return 0;
}

static int lcdl_SetChar(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);
	uint8_t nchar = lua_tonumber(L, 2);

	if(!lua_istable(L, 3))
		luaL_error(L, "SetChar() 3rd argument is expected to be an array of strings");

	cb_screen.Lock(&lcd->storage->primary.obj, false);
	slcd_selLCD.SetCGRAM(lcd->storage, nchar);

	for(size_t i=0; i<lua_rawlen(L,3); i++){
		lua_rawgeti(L, 3, i+1);
		const char *pat = luaL_checkstring(L, -1);

		uint8_t v=0;
		for(;*pat;pat++){
			v <<=1;
			if(*pat!=' ' && *pat!='0')
				v |= 1;
		}
		lua_pop(L,1);

		slcd_selLCD.SendData(lcd->storage, v);
	}
	cb_screen.Unlock(&lcd->storage->primary.obj);

	return 0;
}

static int lcdl_Refresh(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);

	slcd_selLCD.Refresh(lcd->storage);

	return 0;
}

static int lcdl_dump(lua_State *L){
	struct SelLCDScreenLua *lcd = checkSelLCDScreen(L);

	slcd_selLCD.DumpBuffers(lcd->storage);

	return 0;
}

const struct luaL_Reg LCDScreenMethods[] = {
	{"Shutdown", lcdl_Shutdown},
	{"Backlight", lcdl_Backlight},
	{"DisplayCtl", lcdl_DisplayCtl},
	{"EntryCtl", lcdl_EntryCtl},
	{"SetDDRAM", lcdl_SetDDRAM},
	{"SetTiming", lcdl_SetTiming},
	{"SetSize", lcdl_SetSize},
	{"bClear", lcdl_bClear},
	{"bWriteString", lcdl_bWriteString},
	{"SetChar", lcdl_SetChar},
	{"Refresh", lcdl_Refresh},
	{"Dump", lcdl_dump},
	{NULL, NULL}    /* End of definition */
};

static const char * const LuaName(){
	return "SelLCDScreen";
}

static bool lcdscr_Lock(struct SelLCDScreen *scr, bool heavy){
		/* Try to lock the mutex */
	if(pthread_mutex_trylock(&scr->mutex) == EBUSY){
			/* Wait to get the lock */
		pthread_mutex_lock(&scr->mutex);

			/* Lets the previous operation (the only that acquired
			 * the lock), to finish
			 */
		if(heavy)
			usleep(1520);	/* 1.52 ms */
		else
			usleep(37);		/* 37 us */
	}

	return true;
}

static bool lcdscr_Unlock(struct SelLCDScreen *scr){
	pthread_mutex_unlock(&scr->mutex);
	return true;
}

bool lcdscr_AllocBuff(struct SelLCDScreen *scr){
#ifdef DEBUG
	slcd_selLog->Log('D', "Buffers allocation");
#endif

		/* Free existing buffers */
	if(scr->working_buffer){
		free(scr->working_buffer);
		scr->working_buffer = NULL;	/* Only if there is a failure afterward */
	}
	if(scr->screen_buffer){
		free(scr->screen_buffer);
		scr->screen_buffer = NULL;	/* Only if there is a failure afterward */
	}

	scr->working_buffer = malloc(scr->primary.w * scr->primary.h);
	assert(scr->working_buffer);
	scr->screen_buffer =  malloc(scr->primary.w * scr->primary.h);
	assert(scr->screen_buffer);

		/* Only to avoid garbages in this buffer.
		 * As it's very unlikely to feed a screen with 0 (clear() is writing
		 * spaces), it will force a full refresh at first ... unlike
		 * we are Clear()ing first.
		 */
	memset(scr->screen_buffer, 0, scr->primary.w * scr->primary.h);

	return(scr->working_buffer && scr->screen_buffer);
}

void initSLScreenCallBacks(){
	slcd_selCore->initGenericSurfaceCallBacks(&cb_screen);

	cb_screen.LuaObjectName = LuaName;

		/* It's the physical screen, so mostly wrappers to the module */
	cb_screen.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))slcd_selLCD.GetSize;
	cb_screen.Home = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Home;
	cb_screen.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))lcdss_subSurface;
	cb_screen.getPrimary = (void *(*)(struct SelGenericSurface *))lcdss_getPrimary;
	cb_screen.getParent = (void *(*)(struct SelGenericSurface *))lcdss_getParent;

	cb_screen.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))slcd_selLCD.SetCursor;
	cb_screen.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))lcdss_inSurface;
	cb_screen.Clear = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Clear;
	cb_screen.WriteString = (bool (*)(struct SelGenericSurface *, const char *))slcd_selLCD.WriteString;

	cb_screen.Lock = (bool (*)(struct SelGenericSurface *, bool))lcdscr_Lock;		/* Lock on physical screen */
	cb_screen.Unlock = (bool (*)(struct SelGenericSurface *))lcdscr_Unlock;	/* Lock on physical screen */

	cb_screen.AllocateBuffer = (bool (*)(struct SelGenericSurface *))lcdscr_AllocBuff;
	cb_screen.Refresh = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Refresh;
	cb_screen.Dump = (bool (*)(struct SelGenericSurface *))slcd_selLCD.DumpBuffers;
	cb_screen.bSet = (void (*)(struct SelGenericSurface *, const char, struct SelCoordinate *))slcd_selLCD.bSet;
}

void initSelLCDScreen(struct SelLCDScreen *lcd){
		/* Default timings */
	lcd->clock_pulse = 500;
	lcd->clock_process = 4100;

	pthread_mutex_init(&lcd->mutex, NULL);

	lcd->working_buffer = lcd->screen_buffer = NULL;

	initSharedSurface(&lcd->primary,
		NULL,	/* No parent, we're primary */
		0,0,	/* let's guess the size */
		0,0,	/* no margin */
		lcd		/* We are the screen */
	);

	lcd->primary.obj.cb = &cb_screen;
	lcd->primary.obj.cb->AllocateBuffer(&lcd->primary.obj);
}
