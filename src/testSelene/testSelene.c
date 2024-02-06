/* Tests Selene C side functionalities
 *
 * 12/06/2023 LF : v7.00.00 - Improve loadable module mechanisme
 */

#include <dlfcn.h>		/* dlopen(), ... */
#include <string.h>
#include <stdlib.h>		/* exit(), ... */
#include <stdio.h>
#include <assert.h>
#include <libgen.h>		/* dirname(), ... */

#include "Selene/libSelene.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"
#include "Selene/SelMQTT.h"
#include "Selene/SelLua.h"

int main( int ac, char ** av){
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

		/* We still need to do it manually as SeleneCore->loadModule() still can't logging */
	if(!SelLog){
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

	SelLog->initFile("/tmp/selene.log", LOG_FILE|LOG_STDOUT);
	SelLog->Log('I', "Logging to file started");
	SelLog->Log('I', "Selene version : %f", SeleneCore->getVersion());

	SelLog->Log('T', "Not yet ignored");
	SelLog->ignoreList("T");
	SelLog->Log('T', "Message Ignored");
	SelLog->ignoreList(NULL);
	SelLog->Log('T', "Not ignored anymore");

	struct SelLua *SelLua = (struct SelLua *)SeleneCore->loadModule("SelLua", SELLUA_VERSION, &verfound, 'F');
	if(!SelLua)
		exit(EXIT_FAILURE);

	SelLog->Log('D', "SelLua %s : version %u", SelLua ? "found":"not found", verfound);

	struct SelMQTT *SelMQTT = (struct SelMQTT *)SeleneCore->loadModule("SelMQTT", SELMQTT_VERSION, &verfound, 'F');
	if(!SelMQTT)
		exit(EXIT_FAILURE);

	SelLog->Log('D', "SelMQTT %s : version %u", SelMQTT ? "found":"not found", verfound);

	exit(EXIT_SUCCESS);
}

