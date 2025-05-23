/* Demonstration of Selene Average Collection
 *
 * 27/03/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelAverageCollection.h>

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

			/* Load SelCollection module ... using SeleneCore's facilities as logging is enabled */
	struct SelAverageCollection *SelAverageCollection = (struct SelAverageCollection *)SeleneCore->loadModule("SelAverageCollection", SELAVERAGECOLLECTION_VERSION, &verfound, 'F');
	if(!SelAverageCollection)
		exit(EXIT_FAILURE);

		/* Create a collection */
	struct SelAverageCollectionStorage *col = SelAverageCollection->create("simple", 5,7,3,1);
	assert(col);	/* No need of a smart error handling as create() will do it by itself) */

	for(size_t i=1; i<=3; i++)
		SelAverageCollection->push(col, 1, i*1.0);

	SelAverageCollection->module.dump(col);

	lua_Number min,max;
	SelAverageCollection->minmaxIs(col, &min, &max);
	SelLog->Log('D', "immediate min : %lf, max: %lf", min, max);
	SelAverageCollection->minmaxAs(col, &min, &max);
	SelLog->Log('D', "average min : %lf, max: %lf", min, max);

	SelLog->Log('I', "Additional values that eject first ones");

	for(size_t i=5; i<=9; i++)
		SelAverageCollection->push(col, 1, i*1.0);

	SelAverageCollection->module.dump(col);
	SelAverageCollection->minmaxIs(col, &min, &max);
	SelLog->Log('D', "immediate min : %lf, max: %lf", min, max);
	SelAverageCollection->minmaxAs(col, &min, &max);
	SelLog->Log('D', "average min : %lf, max: %lf", min, max);

	SelLog->Log('I', "Fill with random values");

	for(size_t i=1; i<=9; i++)
		SelAverageCollection->push(col, 1, rand()*1.0);

	SelAverageCollection->module.dump(col);
	SelAverageCollection->minmaxIs(col, &min, &max);
	SelLog->Log('D', "immediate min : %lf, max: %lf", min, max);
	SelAverageCollection->minmaxAs(col, &min, &max);
	SelLog->Log('D', "average min : %lf, max: %lf", min, max);

	SelLog->Log('D', "Size i:%ld a:%ld HowMany i:%ld a:%ld", 
		SelAverageCollection->getsizeI(col),
		SelAverageCollection->howmanyI(col),
		SelAverageCollection->getsizeA(col),
		SelAverageCollection->howmanyA(col)
	);

	SelLog->Log('I', "Walk thru gets()");
	for(size_t i=0; i<SelAverageCollection->howmanyI(col); i++)
		SelLog->Log('D', "[%ld] i:%f", i, SelAverageCollection->getsI(col, i));
	for(size_t i=0; i<SelAverageCollection->howmanyA(col); i++)
		SelLog->Log('D', "[%ld] a:%f", i, SelAverageCollection->getsA(col, i));

		/* Test clearing the collection */
	SelLog->Log('I', "Clear");
	SelAverageCollection->clear(col);
	SelAverageCollection->module.dump(col);

	SelLog->Log('D', "Find() ...");

	struct SelAverageCollectionStorage *colf = SelAverageCollection->find("simple", 0);
	if(colf){
		SelAverageCollection->module.dump(colf);
		SelLog->Log('I', "Existant test succeed");
	} else
		SelLog->Log('F', "Existant test failled");

	colf = SelAverageCollection->find("Non existant", 0);
	if(!colf)
		SelLog->Log('I', "Non existant test succeed");
	else
		SelLog->Log('F', "Non existant test failled");

	exit(EXIT_SUCCESS);
}
