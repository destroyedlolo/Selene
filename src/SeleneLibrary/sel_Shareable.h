/* sel_Shareable.h
 *
 * Stuffs having to be added to objects to make them shareable.
 * (internal use only)
 *
 * 07/03/2021 : creation
 */

#ifndef SEL_SHAREABLE_H
#define SEL_SHAREABLE_H

#include <pthread.h>

struct sel_Shareable {
	pthread_mutex_t mutex;	/* Prevent concurrent access */
};

extern void sel_shareable_init( struct sel_Shareable * );
extern void sel_shareable_lock( struct sel_Shareable * );
extern void sel_shareable_unlock( struct sel_Shareable * );
#endif 
