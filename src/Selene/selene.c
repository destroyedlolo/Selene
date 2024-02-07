/* Selene : Automation framework using Lua
 *
 * 07/02/2024 LF : redesign for v7
 */

#include "Selene/libSelene.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"
#include "Selene/SelLua.h"

#include <dlfcn.h>		/* dlerror(), ... */
#include <string.h>
#include <stdlib.h>		/* exit(), ... */
#include <stdio.h>

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
	SelLog->Log('D', "SelLog %s : version %u", SelLog ? "found":"not found", verfound);

		/* After this call, SeleneCore->loadModule() can log errors */
	if(!SeleneCore->SelLogInitialised(SelLog)){
		SelLog->Log('F', "SelLog too old");
		exit(EXIT_FAILURE);
	}

	struct SelLua *SelLua = (struct SelLua *)SeleneCore->loadModule("SelLua", SELLUA_VERSION, &verfound, 'F');
	if(!SelLua)
		exit(EXIT_FAILURE);
	SelLog->Log('D', "SelLua %s : version %u", SelLua ? "found":"not found", verfound);
}
