/* SelAverageCollection.h
 *
 * 11/06/2022 LF : creation
 */

#ifndef SELAVERAGECOLLECTION_H
#define SELAVERAGECOLLECTION_H

#include "libSelene.h"
#include "sel_Shareable.h"

struct imaveragedata {
	lua_Number *data;
};

struct SelAverageCollection {
	struct sel_Shareable shareme;

	unsigned int ndata;	/* how many data per sample */
	unsigned int group;	/* how many sample to average */

	struct imaveragedata *immediate;	/* immediate data */
	unsigned int isize;		/* Length of the data collection */
	unsigned int ilast;		/* Last value pointer */
	char 		ifull;		/* the collection is full */
	unsigned int icidx;		/* Current index for iData() */

	struct imaveragedata *average;	/* Average data */
	unsigned int asize;		/* Length of the data collection */
	unsigned int alast;		/* Last value pointer */
	char 		afull;		/* the collection is full */
	unsigned int acidx;		/* Current index for iData() */
};

extern struct SelAverageCollection **checkSelAverageCollection(lua_State *);

#endif
