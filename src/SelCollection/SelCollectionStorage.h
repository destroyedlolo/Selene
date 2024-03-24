/* SelCollectionStorage.h
 *
 * Collection of values
 */

#ifndef SELCOLLECTIONSTORAGE_H
#define SELCOLLECTIONSTORAGE_H

#include <Selene/SelCollection.h>

struct SelCollectionStorage {
	lua_Number *data;		/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int ndata;	/* how many data per sample */
	unsigned int last;	/* Last value pointer */
	bool full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};

#endif
