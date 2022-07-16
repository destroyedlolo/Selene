/* SelAverageCollection.c
 *
 * Collection of immediate and average values
 *
 * 11/06/2022 LF : creation
 */

#include "SelAverageCollection.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if LUA_VERSION_NUM == 501
#	define lua_rawlen lua_objlen
#endif

#ifdef MCHECK
#	include <mcheck.h>
#else
#	define MCHECK ;
#endif

struct SelAverageCollection **checkSelAverageCollection(lua_State *L){
	void *r = luaL_testudata(L, 1, "SelAverageCollection");
	luaL_argcheck(L, r != NULL, 1, "'SelAverageCollection' expected");
	return (struct SelAverageCollection **)r;
}

static int sacol_create(lua_State *L){
/* Create a new collection
 * -> 1: size of the immediate collection
 * -> 2: size of the average collection
 * -> 3: grouping value
 * -> 4: number of value per sample
 */
	struct SelAverageCollection *col = malloc(sizeof(struct SelAverageCollection));
	struct SelAverageCollection **p = (struct SelAverageCollection **)lua_newuserdata(L, sizeof(struct SelAverageCollection *));
	unsigned int i;

	assert(col);
	assert(p);
	*p = col;

	sel_shareable_init(&col->shareme);

	luaL_getmetatable(L, "SelAverageCollection");
	lua_setmetatable(L, -2);

	if((col->isize = luaL_checkinteger( L, 1 )) <= 0 ){
		fputs("*E* SelAverageCollection's immediate size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	if((col->asize = luaL_checkinteger( L, 2 )) <= 0 ){
		fputs("*E* SelAverageCollection's average size can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

	if((col->group = luaL_checkinteger( L, 3 )) <= 0 ){
		fputs("*E* SelAverageCollection's grouping can't be null or negative\n", stderr);
		exit(EXIT_FAILURE);
	}

/* printf("*D* isz: %d asz: %d grp: %d\n", col->isize, col->asize, col->group); */

	if(col->isize < col->group){
		fputs("*E* SelAverageCollection's grouping can't be > to immediate sample size\n", stderr);
		exit(EXIT_FAILURE);
	}
	
	if((col->ndata = lua_tointeger( L, 4 )) < 1)
		col->ndata = 1;

	assert( (col->immediate = calloc(col->isize, sizeof(struct imaveragedata))) );
	for( i=0; i<col->isize; i++)
		assert( (col->immediate[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->ilast = 0;
	col->ifull = 0;

	assert( (col->average = calloc(col->asize, sizeof(struct imaveragedata))) );
	for( i=0; i<col->asize; i++)
		assert( (col->average[i].data = calloc(col->ndata, sizeof(lua_Number))) );

	col->alast = 0;
	col->afull = 0;

	MCHECK;
	return 1;
}

static int sacol_push(lua_State *L){
/* Arguments are : 
 * 	1 : number ( To be compatible with previous version )
 * 			(only if only 1 value is stored)
 * 		table with the same amount as number of data
 */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	sel_shareable_lock( &(*col)->shareme ); /* Avoid concurrent access during modification */

		/****
		 * Insert immediate value
		 ****/

	if(!lua_istable(L, 2)){	/* One value, old interface */
		if((*col)->ndata > 1){
			sel_shareable_unlock( &(*col)->shareme ); /* Error : the collection needs to be released before raising the error */
			luaL_error(L, "Pushing a single number on multi-valued AverageCollection");
		}

		(*col)->immediate[ (*col)->ilast % (*col)->isize].data[0] = luaL_checknumber( L, 2 );
	} else {	/* Table provided */
		unsigned int j;

		if( lua_rawlen(L,2) != (*col)->ndata ){
			sel_shareable_unlock( &(*col)->shareme );
			luaL_error(L, "Expecting %d data per sample", (*col)->ndata);
		}

		for( j = 0; j < (*col)->ndata; j++){
			lua_rawgeti(L, 2, j+1);
			(*col)->immediate[ (*col)->ilast % (*col)->isize].data[j] = luaL_checknumber( L, -1 );
			lua_pop(L,1);
		}
	}

	if((*col)->ilast++ > (*col)->isize)
		(*col)->ifull = 1;


		/****
		 * Update average values if needed
		 ****/

	if(!((*col)->ilast % (*col)->group)){	/* push a new average */
		unsigned int i;

		if((*col)->ndata > 1){	/* Multi value */
			unsigned int j;
			for( j = 0; j < (*col)->ndata; j++)
				(*col)->average[ (*col)->alast % (*col)->asize].data[j] = 0;
		} else
			(*col)->average[ (*col)->alast % (*col)->asize].data[0] = 0;

		for(i = (*col)->ilast - (*col)->group; i < (*col)->ilast; i++){
			if((*col)->ndata > 1){	/* Multi value */
				unsigned int j;
				for( j = 0; j < (*col)->ndata; j++)
					(*col)->average[(*col)->alast % (*col)->asize].data[j] += (*col)->immediate[i % (*col)->isize].data[j];
			} else
				(*col)->average[(*col)->alast % (*col)->asize].data[0] += (*col)->immediate[i % (*col)->isize].data[0];
		}

		if((*col)->ndata > 1){	/* Multi value */
			unsigned int j;
			for( j = 0; j < (*col)->ndata; j++)
				(*col)->average[(*col)->alast % (*col)->asize].data[j] /= (*col)->group;
		} else
			(*col)->average[ (*col)->alast % (*col)->asize].data[0] /= (*col)->group;

		if((*col)->alast++ > (*col)->asize)
			(*col)->afull = 1;
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 0;
}

static int sacol_minmaxI(lua_State *L){
/* MinMax for immediate values */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[(*col)->ndata], max[(*col)->ndata];

	if(!(*col)->ilast && !(*col)->ifull){
		lua_pushnil(L);
		lua_pushstring(L, "MinMaxI() on an empty collection");
		return 2;
	}

	sel_shareable_lock( &(*col)->shareme );	/* Avoid modification while running through the collection */
  
	ifirst = (*col)->ifull ? (*col)->ilast - (*col)->isize : 0;
	for( j=0; j<(*col)->ndata; j++ )
		min[j] = max[j] = (*col)->immediate[ ifirst % (*col)->isize ].data[j];

	for(i = ifirst; i < (*col)->ilast; i++){
		for( j=0; j<(*col)->ndata; j++ ){
			lua_Number v = (*col)->immediate[ i % (*col)->isize ].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}

	if((*col)->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 2;
}

static int sacol_minmaxA(lua_State *L){
/* MinMax for average values */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[(*col)->ndata], max[(*col)->ndata];

	if(!(*col)->alast && !(*col)->afull){
		lua_pushnil(L);
		lua_pushstring(L, "MinMaxA() on an empty collection");
		return 2;
	}

	sel_shareable_lock( &(*col)->shareme );	/* Avoid modification while running through the collection */
  
	ifirst = (*col)->afull ? (*col)->alast - (*col)->asize : 0;
	for( j=0; j<(*col)->ndata; j++ )
		min[j] = max[j] = (*col)->average[ ifirst % (*col)->asize ].data[j];

	for(i = ifirst; i < (*col)->alast; i++){
		for( j=0; j<(*col)->ndata; j++ ){
			lua_Number v = (*col)->average[ i % (*col)->asize ].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}

	if((*col)->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 2;
}

static int sacol_minmax(lua_State *L){
/* MinMax for both immediate & average value */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int ifirst;	/* First data */
	unsigned int i,j;
	lua_Number min[(*col)->ndata], max[(*col)->ndata];

	if(!(*col)->ilast && !(*col)->ifull){
		lua_pushnil(L);
		lua_pushstring(L, "MinMaxI() on an empty collection");
		return 2;
	}

	sel_shareable_lock( &(*col)->shareme );	/* Avoid modification while running through the collection */
  
	ifirst = (*col)->ifull ? (*col)->ilast - (*col)->isize : 0;
	for( j=0; j<(*col)->ndata; j++ )
		min[j] = max[j] = (*col)->immediate[ ifirst % (*col)->isize ].data[j];

	for(i = ifirst; i < (*col)->ilast; i++){
		for( j=0; j<(*col)->ndata; j++ ){
			lua_Number v = (*col)->immediate[ i % (*col)->isize ].data[j];
			if( v < min[j] )
				min[j] = v;
			if( v > max[j] )
				max[j] = v;
		}
	}

	if((*col)->alast && (*col)->afull){	/* There is something in the average collection */
		ifirst = (*col)->afull ? (*col)->alast - (*col)->asize : 0;

		for(i = ifirst; i < (*col)->alast; i++){
			for( j=0; j<(*col)->ndata; j++ ){
				lua_Number v = (*col)->average[ i % (*col)->asize ].data[j];
				if( v < min[j] )
					min[j] = v;
				if( v > max[j] )
					max[j] = v;
			}
		}
	}

	if((*col)->ndata == 1){
		lua_pushnumber(L, *min);
		lua_pushnumber(L, *max);
	} else {
		lua_newtable(L);	/* min table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, min[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}

		lua_newtable(L);	/* max table */
		for( j=0; j<(*col)->ndata; j++ ){
			lua_pushnumber(L, j+1);		/* the index */
			lua_pushnumber(L, max[j]);	/* the value */
			lua_rawset(L, -3);			/* put in table */
		}
	}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 2;

}

static int sacol_dump(lua_State *L){
/* Debug function */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	unsigned int i,j;

	sel_shareable_lock( &(*col)->shareme );

	printf("SelAverageCollection's Dump (size : %d x %d, last : %d)\nimmediate :\n", (*col)->isize, (*col)->ndata, (*col)->ilast);

	if((*col)->ifull)
		for(i = (*col)->ilast - (*col)->isize; i < (*col)->ilast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->immediate[i % (*col)->isize].data[j]);
			puts("");
		}
	else
		for(i = 0; i < (*col)->ilast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->immediate[i].data[j]);
			puts("");
		}

	puts("Average :");

	if((*col)->afull)
		for(i = (*col)->alast - (*col)->asize; i < (*col)->alast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->average[i % (*col)->asize].data[j]);
			puts("");
		}
	else
		for(i = 0; i < (*col)->alast; i++){
			for(j = 0; j < (*col)->ndata; j++)
				printf("\t%lf", (*col)->average[i].data[j]);
			puts("");
		}

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 0;
}

static int sacol_getsize(lua_State *L){
/* Number of entries than can be stored in this collection 
 *	<- 1: immediate
 *  <- 2: average
 */
	struct SelAverageCollection **col = checkSelAverageCollection(L);

	sel_shareable_lock( &(*col)->shareme );
	lua_pushnumber(L, (*col)->isize);
	lua_pushnumber(L, (*col)->asize);
	sel_shareable_unlock( &(*col)->shareme );

	return 2;
}

static int sacol_HowMany(lua_State *L){
/* Number of entries really stored
 *	<- 1: immediate
 *  <- 2: average
 */
	struct SelAverageCollection **col = checkSelAverageCollection(L);

	sel_shareable_lock( &(*col)->shareme );
	lua_pushnumber(L, (*col)->ifull ? (*col)->isize : (*col)->ilast);
	lua_pushnumber(L, (*col)->afull ? (*col)->asize : (*col)->alast);
	sel_shareable_unlock( &(*col)->shareme );

	return 2;
}

	/* Iterator */
static int sacol_iinter(lua_State *L){
	struct SelAverageCollection **col = (struct SelAverageCollection **)lua_touserdata(L, lua_upvalueindex(1));

	if((*col)->icidx < (*col)->ilast) {

		sel_shareable_lock( &(*col)->shareme );

		if((*col)->ndata == 1)
			lua_pushnumber(L,  (*col)->immediate[ (*col)->icidx % (*col)->isize ].data[0]);
		else {
			unsigned int j;
			lua_newtable(L);	/* table result */
			for( j=0; j<(*col)->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, (*col)->immediate[ (*col)->icidx % (*col)->isize ].data[j]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		(*col)->icidx++;

		sel_shareable_unlock( &(*col)->shareme );

		MCHECK;
		return 1;
	} else
		return 0;	/* No mutex needed as atomic */
}

static int sacol_idata(lua_State *L){
	struct SelAverageCollection **col = checkSelAverageCollection(L);

	if(!(*col)->ilast && !(*col)->ifull)
		return 0;

	sel_shareable_lock( &(*col)->shareme );

	(*col)->icidx = (*col)->ifull ? (*col)->ilast - (*col)->isize : 0;
	lua_pushcclosure(L, sacol_iinter, 1);

	sel_shareable_unlock( &(*col)->shareme );

	return 1;
}

static int sacol_ainter(lua_State *L){
	struct SelAverageCollection **col = (struct SelAverageCollection **)lua_touserdata(L, lua_upvalueindex(1));

	if((*col)->acidx < (*col)->alast) {

		sel_shareable_lock( &(*col)->shareme );

		if((*col)->ndata == 1)
			lua_pushnumber(L,  (*col)->average[ (*col)->acidx % (*col)->asize ].data[0]);
		else {
			unsigned int j;
			lua_newtable(L);	/* table result */
			for( j=0; j<(*col)->ndata; j++ ){
				lua_pushnumber(L, j+1);		/* the index */
				lua_pushnumber(L, (*col)->average[ (*col)->acidx % (*col)->asize ].data[j]);	/* the value */
				lua_rawset(L, -3);			/* put in table */
			}
		}
		(*col)->acidx++;

		sel_shareable_unlock( &(*col)->shareme );

		MCHECK;
		return 1;
	} else
		return 0;	/* No mutex needed as atomic */
}

static int sacol_adata(lua_State *L){
	struct SelAverageCollection **col = checkSelAverageCollection(L);

	if(!(*col)->alast && !(*col)->afull)
		return 0;

	sel_shareable_lock( &(*col)->shareme );

	(*col)->acidx = (*col)->afull ? (*col)->alast - (*col)->asize : 0;
	lua_pushcclosure(L, sacol_ainter, 1);

	sel_shareable_unlock( &(*col)->shareme );

	return 1;
}


	/* Backup / Restore
	 *
	 * /!\ CAUTION if the target immediate collection is SHORTER
	 * than the original one, some data will be lost !
	 */

static int sacol_Save(lua_State *L){
	/* Save data to file
	 * -> 2: file to save
	 * -> 3: true if only average data have to be saved
	 */
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	const char *s = lua_tostring( L, 2 );
	unsigned int i,j;
	bool avonly = false;

	if(lua_isboolean(L,3))
		avonly = lua_toboolean(L, 3);

	FILE *f = fopen( s, "w" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	sel_shareable_lock( &(*col)->shareme );

		/* Write Header */
	fprintf(f, "SaCMV %d %d\n", (*col)->ndata, (*col)->group);

	if(!avonly){
		if((*col)->ifull)
			for(i = (*col)->ilast - (*col)->isize; i < (*col)->ilast; i++){
				fputc('i', f);
				for(j = 0; j < (*col)->ndata; j++)
					fprintf(f, "\t%lf", (*col)->immediate[i % (*col)->isize].data[j]);
				fputs("\n",f);
			}
		else
			for(i = 0; i < (*col)->ilast; i++){
				fputc('i', f);
				for(j = 0; j < (*col)->ndata; j++)
					fprintf(f, "\t%lf", (*col)->immediate[i].data[j]);
				fputs("\n",f);
			}
	}

	if((*col)->afull)
		for(i = (*col)->alast - (*col)->asize; i < (*col)->alast; i++){
			fputc('a', f);
			for(j = 0; j < (*col)->ndata; j++)
				fprintf(f, "\t%lf", (*col)->average[i % (*col)->asize].data[j]);
			fputs("\n",f);
		}
	else
		for(i = 0; i < (*col)->alast; i++){
			fputc('a', f);
			for(j = 0; j < (*col)->ndata; j++)
				fprintf(f, "\t%lf", (*col)->average[i].data[j]);
			fputs("\n",f);
		}

	fclose(f);

	sel_shareable_unlock( &(*col)->shareme );

	MCHECK;
	return 0;
}

static int stcol_Load(lua_State *L){
	struct SelAverageCollection **col = checkSelAverageCollection(L);
	const char *s = lua_tostring( L, -1 );
	unsigned int i,j;

	FILE *f = fopen( s, "r" );
	if(!f){
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	if(!fscanf(f, "SaCMV %d %d", &j, &i)){
		lua_pushnil(L);
		lua_pushstring(L, "Nagic not found");
		fclose(f);
		return 2;
	}

	if(j != (*col)->ndata){
		lua_pushnil(L);
		lua_pushstring(L, "Amount of data doesn't match");
		fclose(f);
		return 2;
	}
	
	if(i != (*col)->group){
		lua_pushnil(L);
		lua_pushstring(L, "This grouping doesn't match");
		fclose(f);
		return 2;
	}

		/* Reading data */
	sel_shareable_lock( &(*col)->shareme );

	for(;;){
		char cat;
		fscanf(f, "\n%c", &cat);
		if(feof(f))
			break;

		if(cat == 'i'){
			for(j = 0; j < (*col)->ndata; j++)
				fscanf(f, "%lf", &(*col)->immediate[(*col)->ilast % (*col)->isize].data[j] );
			(*col)->ilast++;
		} else if(cat == 'a'){
			for(j = 0; j < (*col)->ndata; j++)
				fscanf(f, "%lf", &(*col)->average[(*col)->alast % (*col)->asize].data[j] );
			(*col)->alast++;
		} else {
			sel_shareable_unlock( &(*col)->shareme );
			lua_pushnil(L);
			lua_pushstring(L, "This grouping doesn't match");
			fclose(f);
			return 2;
		}
		
		if((*col)->ilast > (*col)->isize)
		(*col)->ifull = 1;

		if((*col)->alast > (*col)->asize)
			(*col)->afull = 1;
	}

	fclose(f);

	sel_shareable_unlock( &(*col)->shareme );
	
	MCHECK;
	return 0;
}

static int sacol_clear(lua_State *L){
/* Make the list empty */
	struct SelAverageCollection **col = checkSelAverageCollection(L);

	sel_shareable_lock( &(*col)->shareme );
	(*col)->ilast = 0;
	(*col)->ifull = 0;
	(*col)->alast = 0;
	(*col)->afull = 0;
	sel_shareable_unlock( &(*col)->shareme );

	return 0;
}

static const struct luaL_Reg SelAverageColLib [] = {
	{"Create", sacol_create}, 
	{NULL, NULL}
};

static const struct luaL_Reg SelAverageColM [] = {
	{"Push", sacol_push},
	{"MinMaxI", sacol_minmaxI},
	{"MinMaxImmediate", sacol_minmaxI},
	{"MinMaxA", sacol_minmaxA},
	{"MinMaxAverage", sacol_minmaxA},
	{"MinMax", sacol_minmax},
/*	{"Data", scol_data}, */
	{"iData", sacol_idata},
	{"aData", sacol_adata},
	{"GetSize", sacol_getsize},
	{"HowMany", sacol_HowMany},
	{"Save", sacol_Save},
	{"Load", stcol_Load},
	{"dump", sacol_dump},
	{"Clear", sacol_clear},
	{NULL, NULL}
};


int initSelAverageCollection( lua_State *L ){
	libSel_objFuncs( L, "SelAverageCollection", SelAverageColM);
	libSel_libFuncs( L, "SelAverageCollection", SelAverageColLib );

	return 1;
}
