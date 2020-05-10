/* DRMCairo.h
 *
 * This file contains stuffs related to initialisation of DRMCairo plug-in.
 *
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X.
 *
 * 10/05/2020 LF : Creation
 */
#ifndef SDRMCAIRO_H
#define SDRMCAIRO_H

#	ifdef USE_DRMCAIRO

#include "../../SeleneLibrary/libSelene.h"

extern void initDRMCairo(lua_State *);

#	endif
#endif
