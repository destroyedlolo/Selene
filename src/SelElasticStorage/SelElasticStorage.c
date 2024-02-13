/* SelElasticStorage.c
 *
 * Storage that can be enlarged
 *
 * 13/02/2024 - Migrate from v6
 */

#include "Selene/SelElasticStorage.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"

#include <stdlib.h>

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

static void sesc_free( struct elastic_storage *st ){
/**
 * @brief Release elastic storage structure
 *
 * @function free
 */
	pthread_mutex_lock(&st->mutex);

	if(st->name){
		free((void *)st->name);
		st->name = NULL;
	}

	if(st->data)
		free(st->data);
	st->data = NULL;
	st->storage_sz = 0;

	pthread_mutex_unlock(&st->mutex);
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
	
	registerModule((struct SelModule *)&selElasticStorage);

	return true;
}
