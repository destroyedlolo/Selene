/* Storage that can be enlarged
 *
 */
#ifndef ELASTIC_STORAGE_H
#define ELASTIC_STORAGE_H

#include <sys/types.h>

#define CHUNK_SIZE 512

struct elastic_storage {
	struct elastic_storage *next;
	void *data;
	const char *name;
	int H;
	size_t storage_sz;
	size_t data_sz;
};

/* Initialize a new elastic_storage structure 
 * <- 0 in case of error
 */
extern int EStorage_init( struct elastic_storage * );

/* Check if the elastic_storage's data is valide
 * <- 0 if not
 */
extern size_t EStorage_isOK( struct elastic_storage * );

/* Add chuck of data in an existing elastic_storage
 * -> data : data to store
 * -> size : amount of data to store
 * <- 0 if running out of memory otherwise size of the data
 */
extern size_t EStorage_Feed( struct elastic_storage *, const void *data, size_t size);

/* set the name of an elastic_storage
 * -> es : elastic_storage to name
 * -> name : guess what ?
 * -> list : pointer to named elastic_storage list
 * <- 0 if running out of memory
 */
extern int EStorage_SetName( struct elastic_storage *es, const char *name, struct elastic_storage **list);

#endif
