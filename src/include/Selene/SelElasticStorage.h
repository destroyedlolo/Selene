/* SelElasticStorage.h
 *
 * Storage that can be enlarged
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELELASTIC_STORAGE_VERSION
#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELELASTIC_STORAGE_VERSION 2

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
#include <pthread.h>
#include <lua.h>

struct elastic_storage {
	struct elastic_storage *next;
	void *data;
	const char *name;
/*
	int H;
*/
	size_t storage_sz;
	size_t data_sz;
	pthread_mutex_t mutex;
};

struct elastic_storage_SLList {	/* Simple linked list of storage */
	struct elastic_storage *last;	/* Last list element */
	pthread_mutex_t mutex;
};

struct SelElasticStorage {
	struct SelModule module;

		/* Call backs */
	size_t (*init)(struct elastic_storage *);
	void (*free)(struct elastic_storage *);
	size_t (*isOk)(struct elastic_storage *);
	size_t (*Feed)(struct elastic_storage *, const void *, size_t);

	void (*initSLList)(struct elastic_storage_SLList *);
/*	bool (*SetName)(struct elastic_storage *, const char *, struct elastic_storage_SLList *); */
	void (*SetName)(struct elastic_storage *, const char *);

	int (*dumpwriter)(lua_State *L, const void *b, size_t size, void *s);	/* Lua dump writer */
};

#ifdef __cplusplus
}
#endif

#endif
