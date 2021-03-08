#include "elastic_storage.h"
#include "libSelene.h"
#include "SelShared.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int EStorage_init( struct elastic_storage *st ){
	st->next = NULL;
	st->storage_sz = 0;
	st->name = NULL;
	sel_shareable_init( &st->mutex);

	if(!(st->data = malloc( CHUNK_SIZE )))
		return 0;	// Allocation failled

	st->storage_sz = CHUNK_SIZE;
	st->data_sz = 0;

	return CHUNK_SIZE;
}

void EStorage_free( struct elastic_storage *st ){
	sel_shareable_lock( &st->mutex );

	if( st->name ){
		free( (void *)st->name );
		st->name = NULL;
	}

	if( st->data )
		free( st->data );
	st->data = NULL;
	st->storage_sz = 0;

	sel_shareable_unlock( &st->mutex );
}

size_t EStorage_isOK( struct elastic_storage *st ){
	return st->storage_sz;
}

size_t EStorage_Feed( struct elastic_storage *st, const void *data, size_t size){
	sel_shareable_lock( &st->mutex );

	if( !size ){
		sel_shareable_unlock( &st->mutex );
		return st->data_sz;
	}

	if( st->data_sz + size > st->storage_sz ){	/* new allocation needed */
		st->storage_sz += (CHUNK_SIZE > size) ? CHUNK_SIZE : size;
		if( !(st->data = realloc( st->data, st->storage_sz )) ){
			st->storage_sz = 0;
			sel_shareable_unlock( &st->mutex );
			return 0;
		}
	}

	memcpy( st->data + st->data_sz, data, size );

	sel_shareable_unlock( &st->mutex );
	return(st->data_sz += size);
}

int EStorage_SetName( struct elastic_storage *st, const char *n, struct elastic_storage **list ){
	sel_shareable_lock( &st->mutex );

	if(st->name)	/* remove previous name */
		free( (void *)st->name );

	if( !(st->name = strdup(n)) ){	/* Can't dupplicate string */
		sel_shareable_unlock( &st->mutex );
		return 0;
	}
	st->H = SelL_hash(n);

	if(list && !st->next){	/* list provided AND not already part of it */
		sel_shareable_lock( &SharedStuffs.mutex_sfl );
		st->next = *list;
		*list = st;
		sel_shareable_unlock( &SharedStuffs.mutex_sfl );
	}

	sel_shareable_unlock( &st->mutex );
	return 1;
}

