/* SelQueue.c
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 *	07/04/2018	LF : Migrate to Selene v4
 */
#include "SelFIFO.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct SelFIFO *firstFifo = NULL;

static struct SelFIFO **checkSelFIFO(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelFIFO");
	luaL_argcheck(L, r != NULL, 1, "'SelFIFO' expected");
	return (struct SelFIFO **)r;
}

static int sff_create(lua_State *L){
	struct SelFIFO **q = lua_newuserdata(L, sizeof(struct SelFIFO *));
	assert(q);
	luaL_getmetatable(L, "SelFIFO");
	lua_setmetatable(L, -2);

	assert( (*q = malloc(sizeof(struct SelFIFO))) );
	const char *n = luaL_checkstring(L, 1);	/* Name of the Fifo */
	(*q)->h = SelL_hash(n);
	assert( ((*q)->name = strdup(n)) );

	(*q)->next = firstFifo;
	firstFifo = *q;

	(*q)->first = (*q)->last = NULL;
	pthread_mutex_init( &((*q)->mutex), NULL);
	return 1;
}

static int sff_find(lua_State *L){
	const char *n = luaL_checkstring(L, 1);	/* Name of the Fifo */
	int h = SelL_hash(n);
	struct SelFIFO *p;

	for(p = firstFifo; p; p=p->next)
		if( h == p->h && !strcmp(n, p->name) ){
			struct SelFIFO **q = lua_newuserdata(L,sizeof(struct SelFIFO **));
			assert(q);
			*q = p;
			luaL_getmetatable(L, "SelFIFO");
			lua_setmetatable(L, -2);

			return 1;
		}

	return 0;
}


static int sff_pop(lua_State *L){
	struct SelFIFO *q = *checkSelFIFO(L);
	struct SelFIFOCItem *it;

	pthread_mutex_lock(&q->mutex);	/* Ensure no list modification */
	if(!(it = q->first)){	/* Empty queue */
		pthread_mutex_unlock(&q->mutex);	/* Release the list */
		return 0;
	}
	q->first = it->next;
	if(!q->first)	/* It was the last one */
		q->last = NULL;
	pthread_mutex_unlock(&q->mutex);	/* Release the list */

	if( it->type == LUA_TNUMBER )
		lua_pushnumber(L, it->data.n);
	else {
		lua_pushstring(L, it->data.s);
		free(it->data.s);
	}
	lua_pushnumber(L, it->userdt);

	free(it);

	return 2;
}

static int sff_push(lua_State *L){
	struct SelFIFO *q = *checkSelFIFO(L);

	struct SelFIFOCItem *it = (struct SelFIFOCItem *)malloc( sizeof(struct SelFIFOCItem) );
	if(!it){
		lua_pushnil(L);
		lua_pushstring(L, "SelFIFO:Push() - Runing out of memory");
#ifdef DEBUG
		puts("*E* SelFIFO:Push() - Runing out of memory");
#endif
		return 2;
	}

	it->next = NULL;
	if( lua_type(L, 2) == LUA_TNUMBER ){
		it->type = LUA_TNUMBER;
		it->data.n = lua_tonumber(L, 2);
	} else if( lua_type(L, 2) == LUA_TSTRING ){
		it->type = LUA_TSTRING;
		it->data.s = strdup( lua_tostring(L, 2) );
		if(!it->data.s){
			lua_pushnil(L);
			lua_pushstring(L, "SelFIFO:Push() - Runing out of memory");
#ifdef DEBUG
			puts("*E* SelFIFO:Push() - Runing out of memory");
#endif
			free(it);
			return 2;
		}
	} else {
		lua_pushnil(L);
		lua_pushstring(L, "Only Numbers and Strings can be queued");
#ifdef DEBUG
		puts("*E* Only Numbers and Strings can be queued");
#endif
		free(it);
		return 2;
	}

		/* optional user data */
	if( lua_type(L, 3) == LUA_TNUMBER )
		it->userdt = lua_tonumber(L, 3);
	else if( lua_type(L, 3) == LUA_TBOOLEAN )
		it->userdt = (lua_Number)lua_toboolean(L,3);
	else
		it->userdt = 0;
	
	pthread_mutex_lock(&q->mutex);
		/* Inserting the new data */
	if(q->last){
		q->last->next = it;
		q->last = it;
	} else {	/* First one */
		q->first = q->last = it;
	}
	pthread_mutex_unlock(&q->mutex);

	return 0;
}

static int sff_dump(lua_State *L){
	struct SelFIFO *q = *checkSelFIFO(L);
	struct SelFIFOCItem *it;

	pthread_mutex_lock(&q->mutex);	/* Ensure no list modification */
	printf("SelFIFO '%s'(%d) Dump (first: %p, last: %p)\n", q->name, q->h, q->first, q->last);

	for( it = q->first; it; it = it->next ){
		printf("\t%p : ", it);
		if( it->type == LUA_TNUMBER )
			printf("(number) %lf", it->data.n);
		else if( it->type == LUA_TSTRING )
			printf("(string) '%s'", it->data.s);
		else
			printf("(unknown type) %d", it->type);
		printf(" udt:%f n:%p\n", it->userdt, it->next);	
	}

	pthread_mutex_unlock(&q->mutex);	/* Release the list */
	return 0;
}

static int sff_list(lua_State *L){
	struct SelFIFO *p;
	puts("SelFIFO's list");
	for(p = firstFifo; p; p=p->next)
		printf("\t%p '%s'(%d)\n", p, p->name, p->h);

	return 0;
}

static const struct luaL_Reg SelFFLib [] = {
	{"Create", sff_create},
	{"Find", sff_find},
	{"Push2FIFO", sff_push},
	{NULL, NULL}
};

static const struct luaL_Reg SelFFM [] = {
	{"Push", sff_push},
	{"Pop", sff_pop},
/*	{"HowMany", sff_HowMany}, */
	{"dump", sff_dump},
	{"list", sff_list},
	{NULL, NULL}
};


int initSelFIFO( lua_State *L ){	/* CAUTION : called also when thread are created */
	libSel_objFuncs( L, "SelFIFO", SelFFM);
	libSel_libFuncs( L, "SelFIFO", SelFFLib );

	return 1;
}

