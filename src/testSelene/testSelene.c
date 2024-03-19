/* Tests Selene C side functionalities
 *
 * 12/06/2023 LF : v7.00.00 - Improve loadable module mechanisme
 * 05/03/2024 LF : Add SelSharedVar
 */

#include <Selene/libSelene.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>
#include <Selene/SelSharedVar.h>
#include <Selene/SelFIFO.h>

#include <dlfcn.h>		/* dlerror(), ... */
#include <string.h>
#include <stdlib.h>		/* exit(), ... */
#include <stdio.h>


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

	SelLog->configure("/tmp/selene.log", LOG_FILE|LOG_STDOUT);
	SelLog->Log('I', "Logging to file started");
	SelLog->Log('I', "Selene version : %f", SeleneCore->getVersion());

	SelLog->Log('T', "Not yet ignored");
	SelLog->ignoreList("T");
	SelLog->Log('T', "Message Ignored");
	SelLog->ignoreList(NULL);
	SelLog->Log('T', "Not ignored anymore");

		/* ***
		 * SharedVar's stuffs
		 */
	struct SelSharedVar *SelSharedVar = (struct SelSharedVar *)SeleneCore->loadModule("SelSharedVar", SELSHAREDVAR_VERSION, &verfound, 'F');
	if(!SelSharedVar)
		exit(EXIT_FAILURE);
	
	SelSharedVar->setNumber("immortal", 3.14, 0); /* Creates a numerical variable that doesn't expire */
	SelSharedVar->setNumber("1s", 6.28, 1);	/* Creates a numerical variable that will live for 1 second */
	SelSharedVar->setString("hello", "Hello World", 0);	/* a string */
	SelSharedVar->module.dump();

	SelLog->Log('I', "Clearing a variable");
	SelSharedVar->clear("immortal");
	SelSharedVar->module.dump();

		/* in multithread environment, a special care has to be done to lock
		 * variable's content during it is in use.
		 * Otherwise, very bad thing will happen !
		 */
	enum SharedObjType type;
	union SelSharedVarContent val = SelSharedVar->getValue("hello", &type, true);
	switch(type){
	case SOT_NUMBER:
		SelLog->Log('I', "getValue() -> Number : %lf", val.num);
		break;
	case SOT_STRING:
	case SOT_XSTRING:
		SelLog->Log('I', "getValue() -> String : %s", val.str);
		break;
	default:
		SelLog->Log('I', "getValue() -> Unknown variable");
		break;
	}
	SelSharedVar->unlockVariable("hello");	/* mandatory to avoid deadlock */

		/* ***
		 * Fifo 
		 * */
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

	exit(EXIT_SUCCESS);
}

