/* Selene : Automation framework using Lua
 *
 * 07/02/2024 LF : redesign for v7
 */

#include <Selene/libSelene.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelLua.h>
#include <Selene/SelScripting.h>

#include <dlfcn.h>		/* dlerror(), ... */
#include <string.h>
#include <stdlib.h>		/* exit(), ... */
#include <stdio.h>
#include <libgen.h>		/* dirname(), ... */
#include <assert.h>


int main( int ac, char ** av){
	
	/*
	 * Load mandatory plugins 
	 */

	uint16_t verfound;

	struct SeleneCore *SeleneCore = (struct SeleneCore *)loadModule("SeleneCore", SELENECORE_VERSION, &verfound);
#ifdef DEBUG
	printf("*D* SeleneCore %s : version %u\n", SeleneCore ? "found":"not found", verfound);
#endif
	if(!SeleneCore){	/* Needs to do it manually as SeleneCore is ... not loaded */
		printf("*F* : can't load SeleneCore ");
		if(verfound)
			printf("(%u instead of expected %u)\n", verfound, SELENECORE_VERSION);
		else {
			char *err = dlerror();
			if(!err)
				puts(" : missing InitModule() or newer SelModule expected");
			else
				printf("(%s)\n", dlerror());
		}

		exit(EXIT_FAILURE);
	}

	struct SelLog *SelLog = (struct SelLog *)SeleneCore->loadModule("SelLog", SELLOG_VERSION, &verfound, 'F');
	if(!SelLog){ /* Manual as still can't use SelLog */
		printf("*F* : can't load SelLog ");
		if(verfound)
			printf("(%u instead of expected %u)\n", verfound, SELLOG_VERSION);
		else {
			char *err = dlerror();
			if(!err)
				puts(" : missing InitModule() or outdated dependency found");
			else
				printf("(%s)\n", dlerror());
		}

		exit(EXIT_FAILURE);
	}

		/* After this call, SeleneCore->loadModule() can log errors */
	if(!SeleneCore->SelLogInitialised(SelLog)){
		SelLog->Log('F', "SelLog too old");
		exit(EXIT_FAILURE);
	}

	struct SelLua *SelLua = (struct SelLua *)SeleneCore->loadModule("SelLua", SELLUA_VERSION, &verfound, 'F');
	if(!SelLua)
		exit(EXIT_FAILURE);

	/*
	 * This module is not mandatory ... but it implements Selene's engine
	 */
	struct SelScripting *SelScripting = (struct SelScripting *)SeleneCore->loadModule("SelScripting", SELSCRIPTING_VERSION, &verfound, 'F');
	if(!SelScripting)
		exit(EXIT_FAILURE);

	/*
	 * Execute plugin's initialisation function
	 */
	for(struct SelModule *m = modules; m; m = m->next){
		if(m->initLua)
			m->initLua();
	}

#ifdef DEBUG
	SelLog->Log('D', "Lets go ...");
#endif

	/*
	 * Let's go with user Lua scripts
	 */
	char l[1024];

	if(ac > 1){
		if(ac > 2){ /* Handle script's arguments */
			int i;
			luaL_checkstack(SelLua->getLuaState(), ac-1, "too many arguments to script");	/* Place for args (ac-2) + the table itself */
			lua_createtable(SelLua->getLuaState(), ac-2, 0);
			for(i=2; i<ac; i++){
				lua_pushstring(SelLua->getLuaState(), av[i]);
				lua_rawseti(SelLua->getLuaState(), -2, i-1);
			}
			lua_setglobal(SelLua->getLuaState(), "arg");
		}

		char *t = strdup( av[1] );	/* Export script's stuffs */
		assert(t);
		lua_pushstring(SelLua->getLuaState(), dirname(t) );
		lua_setglobal(SelLua->getLuaState(), "SELENE_SCRIPT_DIR");
		strcpy(t, av[1]);
		lua_pushstring(SelLua->getLuaState(), basename(t) );
		lua_setglobal(SelLua->getLuaState(), "SELENE_SCRIPT_NAME");

		int err = luaL_loadfile(SelLua->getLuaState(), av[1]) || lua_pcall(SelLua->getLuaState(), 0, 0, 0);
		if(err){
			fprintf(stderr, "%s", lua_tostring(SelLua->getLuaState(), -1));
			lua_pop(SelLua->getLuaState(), 1);  /* pop error message from the stack */
			exit(EXIT_FAILURE);
		}
	} else while(fgets(l, sizeof(l), stdin) != NULL){	/* Interactive mode */
		int err = luaL_loadbuffer(SelLua->getLuaState(), l, strlen(l), "line") || lua_pcall(SelLua->getLuaState(), 0, 0, 0);
		if(err){
			fprintf(stderr, "%s\n", lua_tostring(SelLua->getLuaState(), -1));
			lua_pop(SelLua->getLuaState(), 1); /* pop error message from the stack */
		}
	}

	exit(EXIT_SUCCESS);
}
