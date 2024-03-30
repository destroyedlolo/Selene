/* SelCollectionStorage.h
 *
 * Collection of values
 */

#ifndef SELCOLLECTIONSTORAGE_H
#define SELCOLLECTIONSTORAGE_H

#include <Selene/SelCollection.h>

#include <pthread.h>

struct SelCollectionStorage {
	pthread_mutex_t mutex;	/* Prevent concurrent access */

	lua_Number *data;		/* Data */
	size_t size;	/* Length of the data collection */
	size_t ndata;	/* how many data per sample */
	size_t last;	/* Last value pointer */
	bool full;			/* the collection is full */
	size_t cidx;	/* Current index for iData() */
};

#endif
