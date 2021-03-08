/* sel_Shareable.h
 *
 * Stuffs having to be added to objects to make them shareable.
 * (internal use only)
 *
 * 07/03/2021 : creation
 */

#include "sel_Shareable.h"

void sel_shareable_init( struct sel_Shareable *s ){
	pthread_mutex_init(&s->mutex,NULL);
}

void sel_shareable_lock( struct sel_Shareable *s ){
	pthread_mutex_lock( &s->mutex );
}

void sel_shareable_unlock( struct sel_Shareable *s ){
	pthread_mutex_unlock( &s->mutex );
}
