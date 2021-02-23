/* SelCollection.h
 *
 * Collection of values
 *
 * 15/02/2021 LF : emancipate to create shared collection
 */

#ifndef SELCOLLECTION_H
#define SELCOLLECTION_H

#include "libSelene.h"

struct SelCollection {
	lua_Number *data;		/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int ndata;	/* how many data per sample */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};


#endif
