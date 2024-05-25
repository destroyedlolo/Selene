/* SelElasticStorage.c
 *
 * Storage that can be enlarged
 *
 * 13/02/2024 - Migrate from v6
 */

#include <Selene/SelElasticStorage.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <stdlib.h>
#include <string.h>

static struct SelElasticStorage selElasticStorage;

static struct SeleneCore *selCore;
static struct SelLog *selLog;

#define CHUNK_SIZE 512

static size_t sesc_init(struct elastic_storage *st){
/**
 * @brief Initialise elastic storage structure
 *
 * @function init
 * @treturn size_t allocated size
 */
	st->next = NULL;
	st->storage_sz = 0;
	st->name = NULL;
	pthread_mutex_init(&st->mutex, NULL);

	if(!(st->data = malloc(CHUNK_SIZE)))
		return 0;	// Allocation failled

	st->storage_sz = CHUNK_SIZE;
	st->data_sz = 0;

	return CHUNK_SIZE;
}

static void sesc_free(struct elastic_storage *st){
/**
 * @brief Release elastic storage structure
 *
 * @function free
 */
	pthread_mutex_lock(&st->mutex);

/*	Not needed : it's only a copy of the SelSharedFunction's name
	if(st->name){
		free((void *)st->name);
		st->name = NULL;
	}
*/

	if(st->data)
		free(st->data);
	st->data = NULL;
	st->storage_sz = 0;

	pthread_mutex_unlock(&st->mutex);
}

static size_t sesc_isOk(struct elastic_storage *st){
/**
 * @brief Check if the elastic_storage's data is valid and contains data
 *
 * @function isOK
 * @treturn size_t 0 if not, otherwise it's size
 */
	return st->storage_sz;
}

static size_t sesc_Feed(struct elastic_storage *st, const void *data, size_t size){
/**
 * @brief Check if the elastic_storage's data is valid and contains data
 *
 * @function isOK
 * @tparam const void * data to be stored
 * @tparam size_t its size
 * @treturn size_t 0 if not, otherwise it's size
 */
	pthread_mutex_lock(&st->mutex);

	if(!size){
		pthread_mutex_unlock(&st->mutex);
		return st->data_sz;
	}

	if(st->data_sz + size > st->storage_sz){	/* new allocation needed */
		st->storage_sz += (CHUNK_SIZE > size) ? CHUNK_SIZE : size;
		if(!(st->data = realloc( st->data, st->storage_sz ))){
			st->storage_sz = 0;
			pthread_mutex_unlock(&st->mutex);
			return 0;
		}
	}

	memcpy(st->data + st->data_sz, data, size);

	pthread_mutex_unlock(&st->mutex);
	return(st->data_sz += size);
}

static void sesc_SetName(struct elastic_storage *st, const char *n){
	st->name = n;
}

#if 0
static bool sesc_SetName(struct elastic_storage *st, const char *n, struct elastic_storage_SLList *list){
/**
 * @brief set the name of an elastic_storage (and add it to a list if provided)
 *
 * @function SetName
 * @tparam const char *name
 * @tparam struct elastic_storage **list to add to
 * @treturn size_t 0 if not, otherwise it's size
 */
	pthread_mutex_lock(&st->mutex);

	if(st->name)	/* remove previous name */
		free((void *)st->name);

	if( !(st->name = strdup(n)) ){	/* Can't dupplicate string */
		pthread_mutex_unlock(&st->mutex);
		return false;
	}
	st->H = selL_hash(n);

	if(list && !st->next){	/* list provided AND not already part of it */
		pthread_mutex_lock(&list->mutex);
		st->next = list->last;
		list->last = st;
		pthread_mutex_unlock(&list->mutex);
	}

	pthread_mutex_unlock( &st->mutex );
	return true;
}
#endif

static int sesc_dumpwriter(lua_State *L, const void *b, size_t size, void *s){
	(void)L;	/* Avoid a warning */
	if(!(selElasticStorage.Feed(s, b, size) ))
		return 1;	/* Unable to allocate some memory */
	
	return 0;
}


static void sesc_initSLList(struct elastic_storage_SLList *list){
/**
 * @brief Initialise a single linked list of storage
 *
 * @function initSLList
 */
	list->last = NULL;
	pthread_mutex_init(&list->mutex, NULL);
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selElasticStorage, "SelElasticStorage", SELELASTIC_STORAGE_VERSION, LIBSELENE_VERSION))
		return false;

	selElasticStorage.init = sesc_init;
	selElasticStorage.free = sesc_free;
	selElasticStorage.isOk = sesc_isOk;
	selElasticStorage.Feed = sesc_Feed;

	selElasticStorage.initSLList = sesc_initSLList;
	selElasticStorage.SetName = sesc_SetName;

	selElasticStorage.dumpwriter = sesc_dumpwriter;
	
	registerModule((struct SelModule *)&selElasticStorage);

	return true;
}
