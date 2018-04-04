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
 */

#include <dlfcn.h>		/* dlopen(), ... */
#include <string.h>
#include <stdlib.h>		/* exit(), ... */
#include <assert.h>
#include <libgen.h>		/* dirname(), ... */

#include <lua.h>
#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */

#include "SeleneLibrary/libSelene.h"

#include "version.h"

int main( int ac, char ** av){
	char l[1024];

	/* Start with Lua */
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushnumber(L, VERSION);	/* Expose version to lua side */
	lua_setglobal(L, "SELENE_VERSION");

	initSelene(L);	/* Declare Selene own functions */
	initSelLog(L);
	initSelCollection(L);
	initSelTimedCollection(L);

	if(ac > 1){
		if(ac > 2){ /* Handle script's arguments */
			luaL_checkstack(L, ac-1, "too many arguments to script");	/* Place for args (ac-2) + the table itself */
			lua_createtable(L, ac-2, 0);
			for(int i=2; i<ac; i++){
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

