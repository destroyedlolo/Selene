/*	SelTimedCollection.c
 *
 *	Timed values collection
 *
 *	10/04/2017	LF : First version
 */

#include "SelTimedCollection.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static struct SelTimedCollection *checkSelTimedCollection(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelTimedCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelTimedCollection' expected");
	return (struct SelTimedCollection *)r;
}

static int stcol_push(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	col->data[ col->last % col->size].data = luaL_checknumber( L, 2 );
	col->data[ col->last++ % col->size].t = (lua_type( L, 3 ) == LUA_TNUMBER) ? lua_tonumber( L, 3 ) : time(NULL);

	if(col->last > col->size)
		col->full = 1;
	return 0;
}

static int stcol_minmax(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);
	float min,max;
	unsigned int ifirst;	/* First data */

	if(!col->last && !col->full){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size : 0;
	min = max = col->data[ ifirst % col->size ].data;

	for(unsigned int i = ifirst; i < col->last; i++){
		if( col->data[ i % col->size ].data < min )
			min = col->data[ i % col->size ].data;
		if( col->data[ i % col->size ].data > max )
			max = col->data[ i % col->size ].data;
	}

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);

	return 2;
}

	/* Number of entries than can be stored in this collection */
static int stcol_getsize(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

	/* Number of entries really stored */
static int stcol_HowMany(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	lua_pushnumber(L, col->full ? col->size : col->last);
	return 1;
}

	/* Iterator */
static int stcol_inter(lua_State *L){
	struct SelTimedCollection *col = (struct SelTimedCollection *)lua_touserdata(L, lua_upvalueindex(1));

	if(col->cidx < col->last) {
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].data);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].t);
		col->cidx++;
		return 2;
	} else
		return 0;
}

static int stcol_idata(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	if(!col->last && !col->full)
		return 0;

	col->cidx = col->full ? col->last - col->size : 0;
	lua_pushcclosure(L, stcol_inter, 1);

	return 1;
}

	/* Backup / Restore */
static int stcol_Save(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);
	const char *s = lua_tostring( L, -1 );

	FILE *f = fopen( s, "w" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	if(col->full)
		for(unsigned int i = col->last - col->size; i < col->last; i++)
			fprintf(f, "%f@%ld\n", col->data[i % col->size].data, col->data[i % col->size].t );
	else
		for(unsigned int i = 0; i < col->last; i++)
			fprintf(f, "%f@%ld\n", col->data[i].data, col->data[col->size].t );

	fclose(f);

	return 0;
}

static int stcol_Load(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);
	const char *s = lua_tostring( L, -1 );
	float d;
	long int t;

	FILE *f = fopen( s, "r" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	while( fscanf(f, "%f@%ld\n", &d, &t) != EOF){
		col->data[ col->last % col->size].data = d;
		col->data[ col->last++ % col->size].t = t;
	}

	if(col->last > col->size)
		col->full = 1;

	fclose(f);

	return 0;
}

	/* Debug function */
static int stcol_dump(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	printf("SelTimedCollection's Dump (size : %d, last : %d)\n", col->size, col->last);
	if(col->full)
		for(unsigned int i = col->last - col->size; i < col->last; i++)
			printf("\t%f @ %s", col->data[i % col->size].data, ctime( &col->data[i % col->size].t ) );
	else
		for(unsigned int i = 0; i < col->last; i++)
			printf("\t%f @ %s", col->data[i].data, ctime( &col->data[i].t ) );
	return 0;
}

static int stcol_create(lua_State *L){
	struct SelTimedCollection *col = (struct SelTimedCollection *)lua_newuserdata(L, sizeof(struct SelTimedCollection));
	luaL_getmetatable(L, "SelTimedCollection");
	lua_setmetatable(L, -2);
	if(!(col->size = luaL_checkint( L, 1 ))){
		fputs("*E* SelTimedCollection's size can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	assert( col->data = calloc(col->size, sizeof(struct timeddata)) );
	col->last = 0;
	col->full = 0;

	return 1;
}

static const struct luaL_reg SelTimedColLib [] = {
	{"create", stcol_create}, 
	{NULL, NULL}
};

static const struct luaL_reg SelTimedColM [] = {
	{"Push", stcol_push},
	{"MinMax", stcol_minmax},
/*	{"Data", scol_data}, */
	{"iData", stcol_idata},
	{"GetSize", stcol_getsize},
	{"HowMany", stcol_HowMany},
	{"Save", stcol_Save},
	{"Load", stcol_Load},
	{"dump", stcol_dump},
	{NULL, NULL}
};


void init_SelTimedCollection( lua_State *L ){
	luaL_newmetatable(L, "SelTimedCollection");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelTimedColM);
	luaL_register(L,"SelTimedCollection", SelTimedColLib);
}
