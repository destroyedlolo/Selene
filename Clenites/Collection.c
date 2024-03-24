/* Demonstration of Selene Collection
 *
 * 24/03/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelCollection.h>

	/* Here start 'standard' C code */
#include <dlfcn.h>		/* dlerror(), ... */
#include <stdlib.h>		/* exit(), ... */
#include <string.h>
#include <assert.h>

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
	
			/* Load SelCollection module ... using SeleneCore's facilities as logging is enabled */
	struct SelCollection *SelCollection = (struct SelCollection *)SeleneCore->loadModule("SelCollection", SELCOLLECTION_VERSION, &verfound, 'F');
	if(!SelCollection)
		exit(EXIT_FAILURE);

		/* Create a new collection with 3 values*/
	struct SelCollectionStorage *col = SelCollection->create(5,3);
	assert(col);	/* No need of a smart error handling as create() will do it by itself) */

		/* CAUTION : Don't forget to push only FLOAT.
		 * There is strictly not portable way to ensure that and if you
		 * do not, in the better case you will lose data, in the worst
		 * one the application will crash.
		 */
	SelCollection->push(col, 3, 1.0, 2.0, 3.1415);

	SelCollection->module.dump(col);
	
	exit(EXIT_SUCCESS);
}
