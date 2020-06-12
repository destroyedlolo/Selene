/* Selene : Automation framework using Lua
 *
 * 12/04/2015 LF : First version
 * 25/04/2015 LF : Use loadfile() for script
 * 07/06/2015 LF : bump to v0.01 as MQTT is finalized
 * 02/07/2015 LF : add Sleep()
 * 03/07/2015 LF : add WaitFor()
 * 18/09/2015 LF : add SELENE_SCRIPT_* global variables
 * 28/09/2015 LF : v0.03.0 - Add Collection
 * 03/10/2015 LF : v0.04.0 - Subscrition function is not mandatory anymore
 * 24/10/2015 LF : v0.05.0 - Add Layer
 * 26/10/2015 LF : v0.06.0 - Add Window
 * 26/10/2015 LF : v0.07.0 - Add TaskOnce different value
 * 24/01/2016 LF : v0.08.0 - Add watchdog to MQTT subscriptions
 * 12/04/2016 LF : switch to v1.0.0
 *
 * 16/04/2016 LF : switch to v2.0.0 - DirectFB is now a plugin
 * 22/04/2016 LF : Remove lua_mutex (not used as MQTT's function use their own state)
 * 04/05/2016 LF : Add Detach()
 *
 * 05/09/2016 LF : switch to v3.0.0 - Add Curses plugin
 * 19/09/2016 LF : v3.1.0 - WaitFor can wait for io stream
 * 03/12/2016 LF : v3.2.0 - SelLog can log on stdout as well
 * 14/12/2016 LF : v3.3.0 - can use unset timer
 * 31/12/2016 LF : v3.4.0 - DFB : Add toSurface() to Image
 * 			  LF : v3.5.0 - DFB : Add blitting
 * 01/12/2016 LF : v3.6.0 - DFB : add clone()/restore()
 * 			  LF : v3.7.0 - DFB : add SetClip() and SurfaceTileBlitClip()
 * 04/02/2017 LF : v3.8.0 - DFB : add DrawCircle()
 * 08/02/2017 LF : v3.9.0 - DFB : add PixelFormat()
 * 11/03/2017 LF : v3.10.0 - DFB : add FillGradient()
 * 24/03/2017 LF : v3.11.0 - Collection : add HowMany()
 * 25/03/2017 LF : v3.12.0 - DFB : Add SetRenderOptions()
 * 06/04/2017 LF : v3.13.0 - DFB : Add GetAfter() & GetBelow()
 * 10/04/2017 LF : v3.14.0 - Add SelTimedCollection
 * 15/04/2017 LF : v3.15.0 - Add SigIntTask()
 * 15/04/2017 LF : v3.16.0 - Add Save() and Load() to SelTimedCollection
 * 						   - SigIntTask() handles SIGUSR1 as well
 * 16/04/2017 LF : v3.17.0 - DFB : add PixelFormat to windows
 * 24/04/2017 LF : v3.18.0 - Add SelEvent
 * 05/06/2017 LF : v3.19.0 - Add SelTimedWindowCollection
 * 16/06/2017 LF : v3.20.0 - Add SelFIFO
 * 16/08/2017 LF : v3.21.0 - Create arg array as Lua is doing
 * 25/08/2017 LF : v3.22.0 - Add SelTimedWindowCollection:DiffMinMax()
 *
 * 03/05/2018 LF : v4.00.0 - Correct multi-threading Lua handling
 * 							Compatible with Lua 5.3.4 as well
 * 04/05/2018 LF : v4.01.0 - Curse plugin ported
 *
 * 26/12/2018 LF : V5.00.00 - Introduce OLED plugin
 * 08/01/2019 LF : v5.01.00 - Add SELPLUG_* variables
 *
 * 10/05/2020 LF : v6.00.00	- Introduce DRMCairo
 * 12/06/2020 LF : v6.01.00	- Add FrameBuffer extension
 */

#include <dlfcn.h>		/* dlopen(), ... */
#include <string.h>
#include <stdlib.h>		/* exit(), ... */
#include <assert.h>
#include <libgen.h>		/* dirname(), ... */

#include "SeleneLibrary/libSelene.h"

#include "version.h"

	/*
	 * Dynamically add Pluggins
	 */

#ifdef USE_DRMCAIRO
static int UseDRMCairo( lua_State *L ){
	void *pgh;
	void (*func)( lua_State * );

	if(!(pgh = dlopen(PLUGIN_DIR "/SelDRMCairo.so", RTLD_LAZY))){
		fprintf(stderr, "Can't load plug-in : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

	if(!(func = dlsym( pgh, "initDRMCairo" ))){
		fprintf(stderr, "Can't find plug-in init function : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	(*func)( L );

	lua_pushboolean(L, true);	/* Expose plugin to lua side */
	lua_setglobal(L, "SELPLUG_DRMCairo");

#	ifdef DRMC_WITH_FB
	lua_pushboolean(L, true);	/* Expose plugin to lua side */
	lua_setglobal(L, "SELPLUG_DRMCairo_FBEXTENSION");
#	endif

	return 0;
}

static const struct luaL_Reg seleneDRMCairoAdditionalLib[] = {
	{"UseDRMCairo", UseDRMCairo},
	{NULL, NULL}    /* End of definition */
};
#endif

#ifdef USE_OLED
static int UseOLED( lua_State *L ){
	void *pgh;
	void (*func)( lua_State * );

	if(!(pgh = dlopen(PLUGIN_DIR "/SelOLED.so", RTLD_LAZY))){
		fprintf(stderr, "Can't load plug-in : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

	if(!(func = dlsym( pgh, "initSelOLED" ))){
		fprintf(stderr, "Can't find plug-in init function : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	(*func)( L );

	lua_pushboolean(L, true);	/* Expose plugin to lua side */
	lua_setglobal(L, "SELPLUG_OLED");

	return 0;
}

static const struct luaL_Reg seleneOLEDAdditionalLib[] = {
	{"UseOLED", UseOLED},
	{NULL, NULL}    /* End of definition */
};
#endif

#ifdef USE_CURSES
static int UseCurses( lua_State *L ){
	void *pgh;
	void (*func)( lua_State * );

	if(!(pgh = dlopen(PLUGIN_DIR "/SelCurses.so", RTLD_LAZY))){
		fprintf(stderr, "Can't load plug-in : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

	if(!(func = dlsym( pgh, "initSelCurses" ))){
		fprintf(stderr, "Can't find plug-in init function : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	(*func)( L );

	lua_pushboolean(L, true);	/* Expose plugin to lua side */
	lua_setglobal(L, "SELPLUG_CURSES");

	return 0;
}

static const struct luaL_Reg seleneCurseAdditionalLib[] = {
	{"UseCurses", UseCurses},
	{NULL, NULL}    /* End of definition */
};
#endif

#ifdef USE_DIRECTFB
int UseDirectFB( lua_State *L ){
	void *pgh;
	void (*func)( lua_State * );

	if(!(pgh = dlopen(PLUGIN_DIR "/SelDirectFB.so", RTLD_LAZY))){
		fprintf(stderr, "Can't load plug-in : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

	if(!(func = dlsym( pgh, "init_directfb" ))){
		fprintf(stderr, "Can't find plug-in init function : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	(*func)( L );

	lua_pushboolean(L, true);	/* Expose plugin to lua side */
	lua_setglobal(L, "SELPLUG_DFB");

	return 0;
}

static const struct luaL_Reg seleneDFBAdditionalLib[] = {
	{"UseDirectFB", UseDirectFB},
	{NULL, NULL}    /* End of definition */
};
#endif

int main( int ac, char ** av){
	char l[1024];

	/* Start with Lua */
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	initSeleneLibrary(L);
	
	lua_pushnumber(L, VERSION);	/* Expose version to lua side */
	lua_setglobal(L, "SELENE_VERSION");

	initSelene(L);	/* Declare Selene own functions */
	initSelLog(L);
	initSelCollection(L);
	initSelTimedCollection(L);
	initSelTimedWindowCollection(L);
	initSelTimer(L);
	initSelShared(L);
	initSelSharedFunc(L);
	initSelFIFO(L);
	initSelEvent(L);
	initSelMQTT(L);

#ifdef USE_DRMCAIRO
	libSel_libAddFuncs(L, "Selene", seleneDRMCairoAdditionalLib);
#endif

#ifdef USE_OLED
	libSel_libAddFuncs(L, "Selene", seleneOLEDAdditionalLib);
#endif

#ifdef USE_CURSES
	libSel_libAddFuncs(L, "Selene", seleneCurseAdditionalLib);
#endif

#ifdef USE_DIRECTFB
	libSel_libAddFuncs(L, "Selene", seleneDFBAdditionalLib);
#endif

	if(ac > 1){
		if(ac > 2){ /* Handle script's arguments */
			int i;
			luaL_checkstack(L, ac-1, "too many arguments to script");	/* Place for args (ac-2) + the table itself */
			lua_createtable(L, ac-2, 0);
			for(i=2; i<ac; i++){
				lua_pushstring(L, av[i]);
				lua_rawseti(L, -2, i-1);
			}
			lua_setglobal(L, "arg");
		}

		char *t = strdup( av[1] );	/* Export script's stuffs */
		assert(t);
		lua_pushstring(L, dirname(t) );
		lua_setglobal(L, "SELENE_SCRIPT_DIR");
		strcpy(t, av[1]);
		lua_pushstring(L, basename(t) );
		lua_setglobal(L, "SELENE_SCRIPT_NAME");

		int err = luaL_loadfile(L, av[1]) || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			exit(EXIT_FAILURE);
		}
	} else while(fgets(l, sizeof(l), stdin) != NULL){	/* Interactive mode */
		int err = luaL_loadbuffer(L, l, strlen(l), "line") || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1); /* pop error message from the stack */
		}
	}

	exit(EXIT_SUCCESS);
}

