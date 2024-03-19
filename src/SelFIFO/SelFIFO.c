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

static struct SelFIFOqueue *firstFifo = NULL;
static pthread_mutex_t mutex;	/* protect concurrent access */

static struct SelFIFOqueue*sfc_find(const char *name, int h){
/** 
 * Find a SelFIFO by its name.
 *
 * @function Find
 * @tparam string name Name of the Fifo
 * @param int hash code (recomputed if null)
 * @treturn ?SelFIFOqueue|nil
 */
	if(!h)
		h = selL_hash(name);

	struct SelFIFOqueue *q;
	pthread_mutex_lock(&mutex);
	for(q = firstFifo; q; q=q->next)
		if(h == q->h && !strcmp(name, q->name)){	/* Found it */
			pthread_mutex_unlock(&mutex);
			return q;
		}
	pthread_mutex_unlock(&mutex);	/* Not found */
	return NULL;
}

static struct SelFIFOqueue*sfc_create(const char *name){
/** 
 * Create or return the existing SelFIFO queue.
 *
 * @function Create
 * @tparam string name Name of the Fifo queue
 */
	int h = selL_hash(name);
	struct SelFIFOqueue *q = sfc_find(name, h);
	if(q)	/* Exists already */
		return q;
	
		/* Create a new one */	
	q = malloc(sizeof(struct SelFIFOqueue));
	assert(q);

		/* Items' list */
	q->first = q->last = NULL;
	pthread_mutex_init(&q->mutex, NULL);

		/* queue's name */
	q->name = strdup(name);
	q->h = h;
	assert(q->name);

		/* links */
	pthread_mutex_lock(&mutex);
	q->next = firstFifo;
	firstFifo = q;
	pthread_mutex_unlock(&mutex);

	return q;
}

static bool sfc_pushS(struct SelFIFOqueue *q, const char *s, lua_Number udata){
/**
 * Push a new item in a queue
 *
 * @function Push
 * @tparam string|number identifier identify the kind of data
 * @tparam ?number|boolean user_data
 */
	struct SelFIFOCItem *it = (struct SelFIFOCItem *)malloc(sizeof(struct SelFIFOCItem));
	if(!it){
		selLog->Log('E', "SelFIFO:Push() - Runing out of memory");
		return false;
	}

	it->next = NULL;
	it->type = LUA_TSTRING;
	it->data.s = strdup(s);
	if(!it->data.s){
		selLog->Log('E', "SelFIFO:Push() - Runing out of memory");
		free(it);
		return false;
	}
	it->userdt = udata;

		/* Inserting the new data */
	pthread_mutex_lock(&q->mutex);
	if(q->last){
		q->last->next = it;
		q->last = it;
	} else {	/* First one */
		q->first = q->last = it;
	}
	pthread_mutex_unlock(&q->mutex);

	return true;
}

static bool sfc_pushN(struct SelFIFOqueue *q, lua_Number n, lua_Number udata){
/**
 * Push a new item in a queue
 *
 * @function Push
 * @tparam string|number identifier identify the kind of data
 * @tparam ?number|boolean user_data
 */
	struct SelFIFOCItem *it = (struct SelFIFOCItem *)malloc(sizeof(struct SelFIFOCItem));
	if(!it){
		selLog->Log('E', "SelFIFO:Push() - Runing out of memory");
		return false;
	}

	it->next = NULL;
	it->type = LUA_TNUMBER;
	it->data.n = n;
	it->userdt = udata;

		/* Inserting the new data */
	pthread_mutex_lock(&q->mutex);
	if(q->last){
		q->last->next = it;
		q->last = it;
	} else {	/* First one */
		q->first = q->last = it;
	}
	pthread_mutex_unlock(&q->mutex);

	return true;
}

static void sfc_dumpqueue(struct SelFIFOqueue *q){
	pthread_mutex_lock(&q->mutex);	/* Ensure no list modification */

	selLog->Log('D', "'%s'(%X) f:%p l:%p", q->name, q->h, q->first, q->last);
	struct SelFIFOCItem *it;
	for(it = q->first; it; it = it->next){
		if(it->type == LUA_TNUMBER)
			selLog->Log('D', "%p : (number) %lf udt:%f n:%p", it, it->data.n, it->userdt, it->next);
		else if(it->type == LUA_TSTRING)
			selLog->Log('D', "%p : (string) \"%s\" udt:%f n:%p", it, it->data.s, it->userdt, it->next);
		else
			selLog->Log('D', "%p : (unknown type) %d udt:%f n:%p", it, it->type, it->userdt, it->next);
	}
	
	pthread_mutex_unlock(&q->mutex);	/* Release the list */
}

static struct SelFIFOCItem *sfc_pop(struct SelFIFOqueue *q){
/** 
 * Pop 1st data
 *
 * @function Pop
 * @return identifier : string or number to identify the kind of data
 * @treturn ?number|boolean user_data
 */
	struct SelFIFOCItem *it;

	pthread_mutex_lock(&q->mutex);		/* Ensure no list modification */

	if(!(it = q->first)){	/* Empty queue */
		pthread_mutex_unlock(&q->mutex);	/* Release the list */
		return NULL;
	}

	q->first = it->next;
	if(!q->first)	/* It was the last one */
		q->last = NULL;

	pthread_mutex_unlock(&q->mutex);	/* Release the list */	

	return(it);
}

static void sfc_freeItem(struct SelFIFOCItem *it){
	if(it->type == LUA_TSTRING)
		free(it->data.s);
	free(it);
}

static bool sfc_isString(struct SelFIFOCItem *it){
	return(it->type == LUA_TSTRING);
}

static bool sfc_isNumber(struct SelFIFOCItem *it){
	return(it->type == LUA_TNUMBER);
}

static const char *sfc_getString(struct SelFIFOCItem *it){
	if(it->type == LUA_TSTRING)
		return(it->data.s);
	else
		return NULL;
}

static lua_Number sfc_getNumber(struct SelFIFOCItem *it){
	if(it->type == LUA_TNUMBER)
		return(it->data.n);
	else
		return 0.0;
}

static lua_Number sfc_getUData(struct SelFIFOCItem *it){
	return(it->userdt);
}

static void sfc_dump(){
	pthread_mutex_lock(&mutex);

	selLog->Log('D', "Dumping FIFO queues list");
	for(struct SelFIFOqueue *q = firstFifo; q; q=q->next)
		sfc_dumpqueue(q);	

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
	selFIFO.find = sfc_find;
 	selFIFO.pushString = sfc_pushS;
 	selFIFO.pushNumber = sfc_pushN;
 	selFIFO.pop = sfc_pop;
 	selFIFO.freeItem = sfc_freeItem;
 	selFIFO.isString = sfc_isString;
 	selFIFO.isNumber = sfc_isNumber;
 	selFIFO.getString = sfc_getString;
 	selFIFO.getNumber = sfc_getNumber;
 	selFIFO.getUData = sfc_getUData;

	selFIFO.dumpQueue = sfc_dumpqueue;

	registerModule((struct SelModule *)&selFIFO);

	pthread_mutex_init(&mutex, NULL);

	return true;
}
