/* SelFIFO.h
 *
 * Versatile FIFO queue
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELFIFO_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELFIFO_VERSION 1

#include <pthread.h>

struct SelFIFOCItem {
	struct SelFIFOCItem *next;
	int type;
	union {
		char *s;
		lua_Number n;
	} data;			/* payload */
	lua_Number userdt;		/* additional (and optional) user data */
};

struct SelFIFOqueue {
	struct SelFIFOCItem *first, *last;
	pthread_mutex_t mutex;	/* prevent concurrent access */

		/* Linked list function */
	struct SelFIFOqueue *next;
	const char *name;
	int h;
};

struct SelFIFO {
	struct SelModule module;

		/* Call backs */
	struct SelFIFOqueue *(*create)(const char *);
	struct SelFIFOqueue *(*find)(const char *, int);
	bool (*pushString)(struct SelFIFOqueue *, const char *, lua_Number);
	bool (*pushNumber)(struct SelFIFOqueue *, lua_Number, lua_Number);
	void (*dumpQueue)(struct SelFIFOqueue *);
};

#endif
