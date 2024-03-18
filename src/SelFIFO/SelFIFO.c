/* SelFIFO.h
 *
 * Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 *	07/04/2018	LF : Migrate to Selene v4
 *	26/06/2020	LF : CAUTION userdt changed from int to lua_Number
 *   ---
 *  18/03/2024	LF : Migrate a Séléné v7's module
 */

#include <Selene/SelFIFO.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct SelFIFO selFIFO;

static struct SeleneCore *selCore;
static struct SelLog *selLog;
static struct SelLua *selLua;

static struct SelFIFOstorage *firstFifo = NULL;
static pthread_mutex_t mutex;	/* protect concurrent access */

static struct SelFIFOstorage *sfc_create(const char *name){
/** 
 * Create a new SelFIFO queue.
 *
 * @function Create
 * @tparam string name Name of the Fifo queue
 */
	struct SelFIFOstorage *q = malloc(sizeof(struct SelFIFOstorage));
	assert(q);

		/* Items' list */
	q->first = q->last = NULL;
	pthread_mutex_init(&q->mutex, NULL);

		/* queue's name */
	q->name = strdup(name);
	q->h = selL_hash(name);
	assert(q->name);

		/* links */
	pthread_mutex_lock(&mutex);
	q->next = firstFifo;
	firstFifo = q;
	pthread_mutex_unlock(&mutex);

	return q;
}

static void sfc_dump(){
	pthread_mutex_lock(&mutex);

	selLog->Log('D', "Dumping FIFO queues list");
	for(struct SelFIFOstorage *q = firstFifo; q; q=q->next){
		selLog->Log('D', "'%s'(%X) f:%p l:%p", q->name, q->h, q->first, q->last);
	}

	pthread_mutex_unlock(&mutex);
}

bool InitModule( void ){
		/* Core modules */
	selCore = (struct SeleneCore *)findModuleByName("SeleneCore", SELENECORE_VERSION);
	if(!selCore)
		return false;

	selLog = (struct SelLog *)selCore->findModuleByName("SelLog", SELLOG_VERSION,'F');
	if(!selLog)
		return false;

		/* Other mandatory modules */

		/* optional modules */
	selLua =  (struct SelLua *)selCore->findModuleByName("SelLua", SELLUA_VERSION,0);

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selFIFO, "SelFIFO", SELFIFO_VERSION, LIBSELENE_VERSION))
		return false;

	selFIFO.module.dump = sfc_dump;
	selFIFO.create = sfc_create;

	registerModule((struct SelModule *)&selFIFO);

	pthread_mutex_init(&mutex, NULL);

	return true;
}
