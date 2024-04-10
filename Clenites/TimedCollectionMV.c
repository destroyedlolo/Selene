/* Demonstration of Selene Multivalued timed Collection
 *
 * 9/04/2024 - First version
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
		 * Multi value collection
		 * ***/
	struct SelTimedCollectionStorage *col = SelTimedCollection->create("Collection", 5,2);
	assert(col);	/* No need of a smart error handling as create() will do it by itself) */

	SelLog->Log('I', "*** Sequential test");
	time_t t = time(NULL);
	for(size_t i=0; i<4; i++)
		SelTimedCollection->push(col, 2, t+i, (lua_Number)i, (lua_Number)(i)-5);
	SelTimedCollection->module.dump(col);

		/* Minmax tests */
	lua_Number mmin[SelTimedCollection->getn(col)], mmax[SelTimedCollection->getn(col)];
	SelTimedCollection->minmax(col, mmin, mmax);
	for(size_t j=0; j<SelTimedCollection->getn(col); j++)
		SelLog->Log('D', "[%ld] min: %lf, max: %lf", j, mmin[j], mmax[j]);

		/* Test using get() */
	SelLog->Log('I', "Walk thru get()");
	for(size_t i=0; i<SelTimedCollection->howmany(col); i++){
		time_t t;
		SelTimedCollection->get(col, &t, i, mmin);
/* Dirty, normally, has to be done dynamically against SelCollection->getn(colm) */
		SelLog->Log('D', "[%ld] %s 0:%f 1:%f", i, SeleneCore->ctime(&t, NULL, 0), mmin[0], mmin[1]);
	}

	SelLog->Log('I', "Walk thru getat()");
}
