/* Demonstration of Selene Average Collection with multiple values
 *
 * 29/03/2024 - First version
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
	struct SelAverageCollectionStorage *col = SelAverageCollection->create(5,7,3,2);
	assert(col);	/* No need of a smart error handling as create() will do it by itself) */

		/* Do some feeding tests */
	for(size_t i=1; i<=3; i++)
		SelAverageCollection->push(col, 2, i*1.0, time(NULL)*1.0-i);
	SelAverageCollection->module.dump(col);	/* Display collection's content */

	SelLog->Log('I', "Additional values that eject first ones");
	for(size_t i=5; i<=9; i++)
		SelAverageCollection->push(col, 2, i*1.0, time(NULL)*1.0-i);
	SelAverageCollection->module.dump(col);

#if 0
	SelLog->Log('I', "Replace with randoms");
	for(size_t i=1; i<=5; i++)
		SelAverageCollection->push(col, 2, rand()*time(NULL)*1.0, rand()*time(NULL)*1.0);
	SelAverageCollection->module.dump(col);
#endif

		/* Minmax tests */
	lua_Number mmin[SelAverageCollection->getn(col)], mmax[SelAverageCollection->getn(col)];
	SelAverageCollection->minmaxI(col, mmin, mmax);
	for(size_t j=0; j<SelAverageCollection->getn(col); j++)
		SelLog->Log('D', "[%ld] Immediate min: %lf, max: %lf", j, mmin[j], mmax[j]);
	SelAverageCollection->minmaxA(col, mmin, mmax);
	for(size_t j=0; j<SelAverageCollection->getn(col); j++)
		SelLog->Log('D', "[%ld] Average min: %lf, max: %lf", j, mmin[j], mmax[j]);

	SelLog->Log('D', "Size i:%ld a:%ld HowMany i:%ld a:%ld", 
		SelAverageCollection->getsizeI(col),
		SelAverageCollection->howmanyI(col),
		SelAverageCollection->getsizeA(col),
		SelAverageCollection->howmanyA(col)
	);

	SelLog->Log('I', "Walk thru gets() - 1st value");
	for(size_t i=0; i<SelAverageCollection->howmanyI(col); i++)
		SelLog->Log('D', "[%ld] i:%f", i, SelAverageCollection->getsI(col, i));
	for(size_t i=0; i<SelAverageCollection->howmanyA(col); i++)
		SelLog->Log('D', "[%ld] a:%f", i, SelAverageCollection->getsA(col, i));

	SelLog->Log('I', "Walk thru get()");
	for(size_t i=0; i<SelAverageCollection->howmanyI(col); i++){
		SelAverageCollection->getI(col, i, mmin);

/* Dirty, normally, has to be done dynamically against SelCollection->getn(colm) */
		SelLog->Log('D', "[%ld] immediate 0:%f 1:%f", i, mmin[0], mmin[1]);
	}
	for(size_t i=0; i<SelAverageCollection->howmanyA(col); i++){
		SelAverageCollection->getA(col, i, mmin);

/* Dirty, normally, has to be done dynamically against SelCollection->getn(colm) */
		SelLog->Log('D', "[%ld] average 0:%f 1:%f", i, mmin[0], mmin[1]);
	}

		/* Test getat() */
	for(size_t i=0; i<SelAverageCollection->howmanyI(col); i++){
		for(size_t j=0; j<SelAverageCollection->getn(col); j++){
			SelLog->Log('I', "immediate [%d,%d] %f", i, j, SelAverageCollection->getatI(col, i, j));
		}
	}
	for(size_t i=0; i<SelAverageCollection->howmanyA(col); i++){
		for(size_t j=0; j<SelAverageCollection->getn(col); j++){
			SelLog->Log('I', "average [%d,%d] %f", i, j, SelAverageCollection->getatA(col, i, j));
		}
	}
		/* Test clearing the collection */
	SelLog->Log('I', "Clear");
	SelAverageCollection->clear(col);
	SelAverageCollection->module.dump(col);
}
