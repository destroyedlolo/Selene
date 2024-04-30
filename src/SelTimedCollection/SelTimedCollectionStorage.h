/* SelTimedCollection.h
 *
 * Collection of timed values
 *
 * 15/02/2021 LF : emancipate to create shared collection
 * 28/03/2024 LF : migrate to v7
 */
#ifndef SELTIMEDCOLLECTION_H
#define SELTIMEDCOLLECTION_H

#include <Selene/SelTimedCollection.h>

#include <pthread.h>

struct timeddata {
	time_t t;
	lua_Number *data;
};

struct SelTimedCollectionStorage {
	struct _SelObject obj;	/* Object management */

	pthread_mutex_t mutex;	/* Prevent concurrent access */

	struct timeddata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int ndata;	/* how many data per sample */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};

#endif
