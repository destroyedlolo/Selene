/* DRMCairo.c
 *
 * This file contains stuffs related to initialisation of DRMCairo plug-in.
 *
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X.
 *
 * 10/05/2020 LF : Creation
 *
 * TODO : for the moment, it deals ONLY with 
 * 	- the 1st available connector
 * 	- the 1st available mode
 */
#ifdef USE_DRMCAIRO

#include "DRMCairo.h"

struct DRMCairoContext DMCContext;

static void cleanDRMC(){
	FT_Done_FreeType(DMCContext.FT);
}

void initDRMCairo(lua_State *L){
	FT_Error err;

	if( (err = FT_Init_FreeType(&DMCContext.FT)) != FT_Err_Ok ){
		fprintf(stderr,"Error %d opening FreeType library.\n", err);
		exit(EXIT_FAILURE);
	}
	atexit(cleanDRMC);

	pthread_mutex_init( &DMCContext.FT_mutex, NULL);

	_include_SelDCCard(L);
	_include_SelDCSurface(L);
	_include_SelFont(L);
}

#endif
