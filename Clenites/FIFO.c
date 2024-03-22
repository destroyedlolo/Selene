/* Demonstration of Selene Shared Variables
 * This example can be used as skeleton of C application using Selene
 *
 * 22/03/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelFIFO.h>

	/* Here start 'standard' C code */
#include <dlfcn.h>		/* dlerror(), ... */
#include <stdlib.h>		/* exit(), ... */
#include <string.h>

int main( int ac, char ** av){
	uint16_t verfound;

		/* Load core functionalities */
	struct SeleneCore *SeleneCore = (struct SeleneCore *)loadModule("SeleneCore", SELENECORE_VERSION, &verfound);
#ifdef DEBUG
	printf("*D* SeleneCore %s : version %u\n", SeleneCore ? "found":"not found", verfound);
#endif
	if(!SeleneCore){	/* Needs to do checks manually as SeleneCore is ... not loaded */
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

		/* ***
		 * Here your application code
		 * ***/
	
			/* Load SelFIFO module ... using SeleneCore's facilities as logging is enabled */
	struct SelFIFO *SelFIFO = (struct SelFIFO *)SeleneCore->loadModule("SelFIFO", SELFIFO_VERSION, &verfound, 'F');
	if(!SelFIFO)
		exit(EXIT_FAILURE);
	
		/* Create a new queue */
	struct SelFIFOqueue *q;
	printf("Queue creation : %p\n", q=SelFIFO->create("Test Queue"));
	printf("Queue reuse    : %p\n", SelFIFO->create("Test Queue"));	/* Try to duplicate it */

	SelFIFO->module.dump();

	SelFIFO->pushString(q, "PI", 3.14);
	SelFIFO->pushNumber(q, 1, 0);
	SelFIFO->module.dump();

		/* Pop queue content */
	struct SelFIFOCItem *it;
	SelLog->Log('I', "Queue content :");
	while((it= SelFIFO->pop(q))){
		if(SelFIFO->isString(it))
			SelLog->Log('I', "String : %s (u: %f)", SelFIFO->getString(it), SelFIFO->getUData(it));
		else if(SelFIFO->isNumber(it))
			SelLog->Log('I', "Number : %f (u: %f)", SelFIFO->getNumber(it), SelFIFO->getUData(it));

		SelFIFO->freeItem(it);
	}

	SelFIFO->module.dump();

	exit(EXIT_SUCCESS);
}
