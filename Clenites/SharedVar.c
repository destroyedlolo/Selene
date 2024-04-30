/* Demonstration of Selene Shared Variables
 * This example can be used as skeleton of C application using Selene
 *
 * 22/03/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelSharedVar.h>

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
	
			/* Load SelSharedVar module ... using SeleneCore's facilities as logging is enabled */
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
		 *
		 * Here we are single threaded
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


	exit(EXIT_SUCCESS);
}
