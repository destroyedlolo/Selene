#include "elastic_storage.h"

#include <stdlib.h>
#include <string.h>

int EStorage_init( struct elastic_storage *st ){
	st->storage_sz = 0;

	if(!(st->data = malloc( CHUNK_SIZE )))
		return 0;	// Allocation failled

	st->storage_sz = CHUNK_SIZE;
	st->data_sz = 0;

	return CHUNK_SIZE;
}

size_t EStorage_isOK( struct elastic_storage *st ){
	return st->storage_sz;
}

size_t EStorage_Feed( struct elastic_storage *st, const void *data, size_t size){
	if( !size )
		return st->data_sz;

	if( st->data_sz + size > st->storage_sz ){	/* new allocation needed */
		st->storage_sz += (CHUNK_SIZE > size) ? CHUNK_SIZE : size;
		if( !(st->data = realloc( st->data, st->storage_sz )) ){
			st->storage_sz = 0;
			return 0;
		}
	}

	memcpy( st->data + st->data_sz, data, size );

	return(st->data_sz += size);
}
