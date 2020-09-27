/*	SelTimedCollection.c
 *
 *	Timed values collection
 *
 *	10/04/2017	LF : First version
 *	24/09/2020	LF : Multivalue
 */

#include "libSelene.h"

#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

struct timeddata {
	time_t t;
	lua_Number *data;
};

struct SelTimedCollection {
	struct timeddata *data;	/* Data */
	unsigned int size;	/* Length of the data collection */
	unsigned int ndata;	/* how many data per sample */
	unsigned int last;	/* Last value pointer */
	char full;			/* the collection is full */
	unsigned int cidx;	/* Current index for iData() */
};

static struct SelTimedCollection *checkSelTimedCollection(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelTimedCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelTimedCollection' expected");
	return (struct SelTimedCollection *)r;
}

static int stcol_create(lua_State *L){
	struct SelTimedCollection *col = (struct SelTimedCollection *)lua_newuserdata(L, sizeof(struct SelTimedCollection));
	assert(col);

	unsigned int i;

	luaL_getmetatable(L, "SelTimedCollection");
	lua_setmetatable(L, -2);

	if((col->size = luaL_checkinteger( L, 1 )) < 0 ){
		fputs("*E* SelTimedCollection's size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	if((col->ndata = lua_tointeger( L, 2 )) < 1)
		col->ndata = 1;

	assert( (col->data = calloc(col->size, sizeof(struct timeddata))) );
	for( i=0; i<col->size; i++)
		assert( (col->data[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->last = 0;
	col->full = 0;

	MCHECK;
	return 1;
}

static int stcol_push(lua_State *L){
/* Arguments are : 
 * 	1 : number ( To be compatible with previous version )
 * 			(only if only 1 value is stored)
 * 		table with the same amount as number of data
 * 	2: timestamp
 * 			if nil, current timestamp
 */
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	if(!lua_istable(L, 2)){	/* One value, old interface */
		if(col->ndata > 1)
			luaL_error(L, "Pushing a single number on multi-valued TimedCollection");

		col->data[ col->last % col->size].data[0] = luaL_checknumber( L, 2 );
	} else {	/* Table provided */
		unsigned int j;

		if( luaL_getn(L,2) != col->ndata )
			luaL_error(L, "Expecting %d data", col->ndata);

		for( j=0; j<col->ndata; j++){
			lua_rawgeti(L, 2, j+1);
			col->data[ col->last % col->size].data[j] = luaL_checknumber( L, -1 );
			lua_pop(L,1);
		}
	}

	col->data[ col->last++ % col->size].t = (lua_type( L, 3 ) == LUA_TNUMBER) ? lua_tonumber( L, 3 ) : time(NULL);

	if(col->last > col->size)
		col->full = 1;

	MCHECK;
	return 0;
}

static int stcol_minmax(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[col->ndata], max[col->ndata];

	if(!col->last && !col->full){
		lua_pushnil(L);
		lua_pushstring(L, "MinMax() on an empty collection");
		return 2;
	}

	ifirst = col->full ? col->last - col->size : 0;
	for( j=0; j<col->ndata; j++ )
		min[j] = max[j] = col->data[ ifirst % col->size ].data[j];

	for(i = ifirst; i < col->last; i++){
		for( j=0; j<col->ndata; j++ ){
			lua_Number v = col->data[ i % col->size ].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}

	if(col->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<col->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<col->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	MCHECK;
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
		if(col->ndata == 1)
			lua_pushnumber(L,  col->data[ col->cidx % col->size ].data[0]);
		else {
			unsigned int j;
			lua_newtable(L);	/* table result */
			for( j=0; j<col->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, col->data[ col->cidx % col->size ].data[j]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		lua_pushnumber(L,  col->data[ col->cidx % col->size ].t);
		col->cidx++;

		MCHECK;
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
	unsigned int i,j;

	FILE *f = fopen( s, "w" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

		/* Write Header */
	fprintf(f, "StCMV %d\n", col->ndata);
	if(col->full)
		for(i = col->last - col->size; i < col->last; i++){
			fprintf(f, "@%ld\n", col->data[i % col->size].t);
			for(j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->data[i % col->size].data[j]);
			fputs("\n",f);
		}
	else
		for(i = 0; i < col->last; i++){
			fprintf(f, "@%ld\n", col->data[i].t);
			for(j = 0; j < col->ndata; j++)
				fprintf(f, "\t%lf", col->data[i].data[j]);
			fputs("\n",f);
		}

	fclose(f);

	MCHECK;
	return 0;
}

static int stcol_Load(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);
	const char *s = lua_tostring( L, -1 );
	long int t;
	unsigned int j;

	FILE *f = fopen( s, "r" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}


	if(!fscanf(f, "StCMV %d", &j)){
		lua_pushnil(L);
		lua_pushstring(L, "Nagic not found");
		fclose(f);
		return 2;
	}

	if(j != col->ndata){
		lua_pushnil(L);
		lua_pushstring(L, "Amount of data doesn't match");
		fclose(f);
		return 2;
	}

	for(;;){
		fscanf(f, "\n@%ld\n", &t);
		if(feof(f))
			break;
		col->data[ col->last % col->size].t = t;
		for(j = 0; j < col->ndata; j++){
			fscanf(f, "%lf", &col->data[col->last % col->size].data[j] );
		}
		col->last++;
	}

	if(col->last > col->size)
		col->full = 1;

	fclose(f);

	MCHECK;
	return 0;
}

	/* Debug function */
static int stcol_dump(lua_State *L){
	struct SelTimedCollection *col = checkSelTimedCollection(L);
	unsigned int i,j;

	printf("SelTimedCollection's Dump (size : %d x %d, last : %d)\n", col->size, col->ndata, col->last);

	if(col->full)
		for(i = col->last - col->size; i < col->last; i++){
			printf( ctime( &col->data[i % col->size].t ) );
			for(j = 0; j < col->ndata; j++)
				printf("\t%lf", col->data[i % col->size].data[j]);
			puts("");
		}
	else
		for(i = 0; i < col->last; i++){
			printf( ctime( &col->data[i].t ) );
			for(j = 0; j < col->ndata; j++)
				printf("\t%lf", col->data[i].data[j]);
			puts("");
		}

	MCHECK;
	return 0;
}

static int stcol_clear(lua_State *L){
/* Make the list empty */
	struct SelTimedCollection *col = checkSelTimedCollection(L);

	col->last = 0;
	col->full = 0;

	return 0;
}

static const struct luaL_Reg SelTimedColLib [] = {
	{"Create", stcol_create}, 
#ifdef COMPATIBILITY
	{"create", stcol_create},
#endif
	{NULL, NULL}
};

static const struct luaL_Reg SelTimedColM [] = {
	{"Push", stcol_push},
	{"MinMax", stcol_minmax},
/*	{"Data", scol_data}, */
	{"iData", stcol_idata},
	{"GetSize", stcol_getsize},
	{"HowMany", stcol_HowMany},
	{"Save", stcol_Save},
	{"Load", stcol_Load},
	{"dump", stcol_dump},
	{"Clear", stcol_clear},
	{NULL, NULL}
};


int initSelTimedCollection( lua_State *L ){
	libSel_objFuncs( L, "SelTimedCollection", SelTimedColM);
	libSel_libFuncs( L, "SelTimedCollection", SelTimedColLib );

	return 1;
}
