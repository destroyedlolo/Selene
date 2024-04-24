/* Demonstration of Selene timed window collection
 *
 * 15/04/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelTimedWindowCollection.h>

	/* Here start 'standard' C code */
#include <dlfcn.h>		/* dlerror(), ... */
#include <stdlib.h>		/* exit(), ... */
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

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
	
			/* Load SelTimedWindowCollection module ... using SeleneCore's facilities as logging is enabled */
	struct SelTimedWindowCollection *SelTimedWindowCollection = (struct SelTimedWindowCollection *)SeleneCore->loadModule("SelTimedWindowCollection", SELTIMEDWINDOWCOLLECTION_VERSION, &verfound, 'F');
	if(!SelTimedWindowCollection)
		exit(EXIT_FAILURE);

	/* create a collection of 5 records,
	 * each of them grouping by 10 seconds
	 */

	struct SelTimedWindowCollectionStorage *col = SelTimedWindowCollection->create("Collection", 5,7);
	assert(col);	/* No need of a smart error handling as create() will do it by itself) */
	
	SelLog->Log('I', "*** Test with an empty collection");
	SelTimedWindowCollection->module.dump(col);

	SelLog->Log('I', "*** Feed with data");
	for(lua_Number i=0; i<3; i++){
		if(i)
			sleep(1);
		SelTimedWindowCollection->push(col, i, 0);
	}
	SelTimedWindowCollection->module.dump(col);

	SelLog->Log('I', "*** Feed with data in the futur");
	time_t t = time(NULL) + 10;
	for(lua_Number i=0; i<15; i++){
		printf("%lf\r", i);
		if(i)
			sleep(1);
		SelTimedWindowCollection->push(col, i, t);
	}
	SelTimedWindowCollection->module.dump(col);

	lua_Number min, max, avg;
	double diff;
	SelTimedWindowCollection->minmax(col, &min, &max, &avg, &diff);
}
