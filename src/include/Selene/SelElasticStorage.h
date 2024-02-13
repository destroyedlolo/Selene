/* SelElasticStorage.h
 *
 * Storage that can be enlarged
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELELASTIC_STORAGE_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELELASTIC_STORAGE_VERSION 1

#include <sys/types.h>
#include <pthread.h>

struct elastic_storage {
	struct elastic_storage *next;
	void *data;
	const char *name;
	int H;
	size_t storage_sz;
	size_t data_sz;
	pthread_mutex_t mutex;
};

struct SelElasticStorage {
	struct SelModule module;

		/* Call backs */
	size_t (*init)(struct elastic_storage *);
	void (*free)(struct elastic_storage *);

};

#ifdef __cplusplus
}
#endif

#endif
