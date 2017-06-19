/* SelQueue.c
 *
 *	Versatile FIFO queue
 *
 *	17/06/2017	LF : First version
 */
#include "SelQueue.h"

#include <assert.h>
#include <stdlib.h>

static struct SelQueue *checkSelQueue(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelQueue");
	luaL_argcheck(L, r != NULL, 1, "'SelQueue' expected");
	return (struct SelQueue *)r;
}

static int sq_create(lua_State *L){
	struct SelQueue *q = (struct SelQueue *)lua_newuserdata(L, sizeof(struct SelQueue));
	assert(q);
	luaL_getmetatable(L, "SelQueue");
	lua_setmetatable(L, -2);

	q->first = q->last = NULL;
	pthread_mutex_init( &(q->mutex), NULL);
	return 1;
}

static int sq_push(lua_State *L){
	struct SelQueue *q = checkSelQueue(L);

	struct SelQCItem *it = (struct SelQCItem *)malloc( sizeof(struct SelQCItem) );
	if(!it){
		lua_pushnil(L);
		lua_pushstring(L, "SelQueue:Push() - Runing out of memory");
#ifdef DEBUG
		puts("*E* SelQueue:Push() - Runing out of memory");
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
			lua_pushstring(L, "SelQueue:Push() - Runing out of memory");
#ifdef DEBUG
			puts("*E* SelQueue:Push() - Runing out of memory");
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

static int sq_dump(lua_State *L){
	struct SelQueue *q = checkSelQueue(L);

	pthread_mutex_lock(&q->mutex);	/* Ensure no list modification */
	printf("SelQueue's Dump (first: %p, last: %p)\n", q->first, q->last);

	for( struct SelQCItem *it = q->first; it; it = it->next ){
		printf("\t%p : ", it);
		if( it->type == LUA_TNUMBER )
			printf("(number) %lf", it->data.n);
		else if( it->type == LUA_TSTRING )
			printf("(string) '%s'", it->data.s);
		else
			printf("(unknown type) %d", it->type);
		printf(" n:%p\n", it->next);	
	}

	pthread_mutex_unlock(&q->mutex);
	return 0;
}

static const struct luaL_reg SelQLib [] = {
	{"create", sq_create},
	{NULL, NULL}
};

static const struct luaL_reg SelQM [] = {
	{"Push", sq_push},
/*	{"HowMany", sq_HowMany}, */
	{"dump", sq_dump},
	{NULL, NULL}
};


void init_SelQueue( lua_State *L ){
	luaL_newmetatable(L, "SelQueue");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelQM);
	luaL_register(L,"SelQueue", SelQLib);
}

