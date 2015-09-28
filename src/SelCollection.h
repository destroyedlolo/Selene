/*	Collection.h
 *
 *	Values collection
 *
 *	28/09/2015	LF : First version
 */

#ifndef SELCOLLECTION_H
#define SELCOLLECTION_H

#include "selene.h"

void init_SelCollection( lua_State * );

struct SelCollection {
	float **data;		/* Data */
	unsigned int len;	/* Length of the data collection */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
};

#endif
