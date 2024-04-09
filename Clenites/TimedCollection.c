/* Demonstration of Selene timed Collection
 *
 * 28/03/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelTimedCollection.h>

	/* Here start 'standard' C code */
#include <dlfcn.h>		/* dlerror(), ... */
#include <stdlib.h>		/* exit(), ... */
#include <string.h>
#include <assert.h>
#include <time.h>

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
	
	srand(time(NULL));	/* Random ... random seed */

			/* Load SelTimedCollection module ... using SeleneCore's facilities as logging is enabled */
	struct SelTimedCollection *SelTimedCollection = (struct SelTimedCollection *)SeleneCore->loadModule("SelTimedCollection", SELTIMEDCOLLECTION_VERSION, &verfound, 'F');
	if(!SelTimedCollection)
		exit(EXIT_FAILURE);


		/* ***
		 * single value collection
		 * ***/
	struct SelTimedCollectionStorage *col = SelTimedCollection->create("Collection", 5,1);
	assert(col);	/* No need of a smart error handling as create() will do it by itself) */

	SelLog->Log('I', "*** Single value test");
	time_t t = time(NULL);
	for(size_t i=0; i<5; i++)
		SelTimedCollection->push(col, 1, t+i, i*1.0);
	SelTimedCollection->module.dump(col);

	lua_Number min,max;
	SelTimedCollection->minmaxs(col, &min, &max);
	SelLog->Log('D', "min: %f, max:%f", min, max);
	SelLog->Log('D', "size: %d(%d), stored: %d", SelTimedCollection->getsize(col), SelTimedCollection->getn(col), SelTimedCollection->howmany(col));

	SelLog->Log('I', "Walk thru");
	for(size_t i=0; i<SelTimedCollection->howmany(col); i++){
		time_t t;
		lua_Number v = SelTimedCollection->gets(col, &t, i);
		SelLog->Log('D', "[%ld] %f (%s)", i, v, SeleneCore->ctime(&t, NULL, 0));
	}

	SelLog->Log('I', "*** Additional values that eject first ones (with the current timestamp");
	for(size_t i=5; i<8; i++)
		SelTimedCollection->push(col, 1, 0, i*1.0);
	SelTimedCollection->module.dump(col);

	SelTimedCollection->minmaxs(col, &min, &max);
	SelLog->Log('D', "min: %f, max:%f", min, max);

	SelLog->Log('I', "*** Replace with randoms");

	for(size_t i=0; i<5; i++)
		SelTimedCollection->push(col, 1, 0, rand()*1.0);
	SelTimedCollection->module.dump(col);

	SelTimedCollection->minmaxs(col, &min, &max);
	SelLog->Log('D', "min: %f, max:%f", min, max);

}
