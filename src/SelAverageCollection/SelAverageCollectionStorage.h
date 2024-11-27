/* SelAverageCollection.h
 */

#ifndef SELAVERAGECOLLECTION_H
#define SELAVERAGECOLLECTION_H

#include "Selene/SelAverageCollection.h"

#include <pthread.h>

struct imaveragedata {
	lua_Number *data;
};

struct SelAverageCollectionStorage {
	struct _SelNamedObject obj;	/* Object management */

	pthread_mutex_t mutex;	/* Prevent concurrent access */

	size_t	ndata;	/* how many data per sample */
	size_t	group;	/* how many sample to average */

	struct	imaveragedata *immediate;	/* immediate data */
	size_t	isize;		/* Length of the data collection */
	size_t	ilast;		/* Last value pointer */
	bool 	ifull;		/* the collection is full */
	size_t	icidx;		/* Current index for iData() */

	struct	imaveragedata *average;	/* Average data */
	size_t	asize;		/* Length of the data collection */
	size_t	alast;		/* Last value pointer */
	bool	afull;		/* the collection is full */
	size_t	acidx;		/* Current index for iData() */
};

#endif
