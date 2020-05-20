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

void initDRMCairo(lua_State *L){
	_include_SelDCCard(L);
	_include_SelDCSurface(L);
	_include_SelFontFace(L);
}

#endif
