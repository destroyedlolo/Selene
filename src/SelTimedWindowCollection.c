/*	SelTimedWindowCollection.c
 *
 *	Timed values collection
 *
 *	10/04/2017	LF : First version
 */

#include "SelTimedWindowCollection.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static struct SelTimedWindowCollection *checkSelTimedWindowCollection(lua_State *L){
	void *r = luaL_checkudata(L, 1, "SelTimedWindowCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelTimedWindowCollection' expected");
	return (struct SelTimedWindowCollection *)r;
}

/* The current implementation rely on :
 * - time_t is an integer kind of
 * - it represents the number of seconds since era
 */
static inline int secw( struct SelTimedWindowCollection *col, time_t t ){
	return( t/col->group );
}

static void stwcol_new(lua_State *L, struct SelTimedWindowCollection *col, lua_Number amin, lua_Number amax, time_t t){
	col->last++;
	if(col->last > col->size)
		col->full = 1;

	col->data[ col->last % col->size].min_data = amin;
	col->data[ col->last % col->size].max_data = amax;
	col->data[ col->last % col->size].t = secw( col, t );
}

static void stwcol_insert(lua_State *L, struct SelTimedWindowCollection *col, lua_Number amin, lua_Number amax, time_t t){
	if(col->last == (unsigned int)-1)	/* Empty collection : create the 1st record */
		stwcol_new( L, col, amin, amax, t );
	else {
		int i = col->last % col->size;
		if( col->data[i].t == secw( col, t ) ){
			if( col->data[i].min_data > amin )
				col->data[i].min_data = amin;
			if( col->data[i].max_data < amax )
				col->data[i].max_data = amax;
		} else
			stwcol_new( L, col, amin, amax, t );
	}
}

static int stwcol_push(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);
	lua_Number dt = luaL_checknumber( L, 2 );

	stwcol_insert(L, col, dt, dt, (lua_type( L, 3 ) == LUA_TNUMBER) ? lua_tonumber( L, 3 ) : time(NULL));

	return 0;
}

static int stwcol_minmax(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);
	lua_Number min,max;
	unsigned int ifirst;	/* First data */

	if(col->last == (unsigned int)-1){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size +1 : 0;
	min = col->data[ ifirst % col->size ].min_data;
	max = col->data[ ifirst % col->size ].max_data;

	for(unsigned int i = ifirst; i <= col->last; i++){
		if( col->data[ i % col->size ].min_data < min )
			min = col->data[ i % col->size ].min_data;
		if( col->data[ i % col->size ].max_data > max )
			max = col->data[ i % col->size ].max_data;
	}

	lua_pushnumber(L, min);
	lua_pushnumber(L, max);

	return 2;
}

	/* Number of entries than can be stored in this collection */
static int stwcol_getsize(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);

	lua_pushnumber(L, col->size);
	return 1;
}

	/* Number of entries really stored */
static int stwcol_HowMany(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);

	lua_pushnumber(L, col->full ? col->size : col->last+1);
	return 1;
}

	/* Iterator */
static int stwcol_inter(lua_State *L){
	struct SelTimedWindowCollection *col = (struct SelTimedWindowCollection *)lua_touserdata(L, lua_upvalueindex(1));

	if(col->cidx <= col->last) {
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].min_data);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].max_data);
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].t * col->group);
		col->cidx++;
		return 3;
	} else
		return 0;
}

static int stwcol_idata(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);

	if(col->last == (unsigned int)-1)
		return 0;

	col->cidx = col->full ? col->last - col->size +1 : 0;
	lua_pushcclosure(L, stwcol_inter, 1);

	return 1;
}

	/* Backup / Restore */
static int stwcol_Save(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);
	const char *s = lua_tostring( L, -1 );

	FILE *f = fopen( s, "w" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	if(col->full)
		for(unsigned int j = col->last - col->size +1; j <= col->last; j++){
			int i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			fprintf(f, "%f/%f@%ld\n", col->data[i].min_data, col->data[i].max_data, t);
		}
	else
		for(unsigned int i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			fprintf(f,"%f/%f@%ld\n", col->data[i].min_data, col->data[i].max_data, t );
		}
	fclose(f);

	return 0;
}

static int stwcol_Load(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);
	const char *s = lua_tostring( L, -1 );
	lua_Number di,da;
	long int t;
	FILE *f = fopen( s, "r" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	while( fscanf(f, "%f/%f@%ld\n", &di, &da, &t) != EOF)
		stwcol_insert( L, col, di, da, t );

	fclose(f);

	return 0;
}

	/* Debug function */
static int stwcol_dump(lua_State *L){
	struct SelTimedWindowCollection *col = checkSelTimedWindowCollection(L);

	printf("SelTimedWindowCollection's Dump (size : %d, last : %d) %s\n", col->size, col->last, col->full ? "Full":"Incomplet");
	if(col->full)
		for(unsigned int j = col->last - col->size +1; j <= col->last; j++){
			int i = j % col->size;
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			printf("\t%f / %f @ %s", col->data[i].min_data, col->data[i].max_data, ctime( &t ) );
		}
	else
		for(unsigned int i = 0; i <= col->last; i++){
			time_t t = col->data[i].t * col->group; /* See secw()'s note */
			printf("\t%f / %f @ %s", col->data[i].min_data, col->data[i].max_data, ctime( &t ) );
		}
	return 0;
}

static int stwcol_create(lua_State *L){
	struct SelTimedWindowCollection *col = (struct SelTimedWindowCollection *)lua_newuserdata(L, sizeof(struct SelTimedWindowCollection));
	luaL_getmetatable(L, "SelTimedWindowCollection");
	lua_setmetatable(L, -2);
	if(!(col->size = luaL_checkint( L, 1 ))){
		fputs("*E* SelTimedWindowCollection's size can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	if(!(col->group = luaL_checkint( L, 2 ))){
		fputs("*E* SelTimedWindowCollection's group can't be null\n", stderr);
		exit(EXIT_FAILURE);
	}
	assert( col->data = calloc(col->size, sizeof(struct timedwdata)) );
	col->last = (unsigned int)-1;
	col->full = 0;

	return 1;
}

static const struct luaL_reg SelTimedColLib [] = {
	{"create", stwcol_create}, 
	{NULL, NULL}
};

static const struct luaL_reg SelTimedColM [] = {
	{"Push", stwcol_push},
	{"MinMax", stwcol_minmax},
/*	{"Data", scol_data}, */
	{"iData", stwcol_idata},
	{"GetSize", stwcol_getsize},
	{"HowMany", stwcol_HowMany},
	{"Save", stwcol_Save},
	{"Load", stwcol_Load},
	{"dump", stwcol_dump},

	{NULL, NULL}
};


void init_SelTimedWindowCollection( lua_State *L ){
	luaL_newmetatable(L, "SelTimedWindowCollection");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */
	luaL_register(L, NULL, SelTimedColM);
	luaL_register(L,"SelTimedWindowCollection", SelTimedColLib);
}
