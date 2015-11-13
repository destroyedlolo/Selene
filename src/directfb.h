/* directfb.h
 *
 * All shared definitions related to directFB
 *
 * 13/04/2015 LF : First version
 */

#ifndef SDIRECTFB_H
#define SDIRECTFB_H

#	ifdef USE_DIRECTFB

#define _POSIX_C_SOURCE 199309	/* Otherwise some defines/types are not defined with -std=c99 */

#include "selene.h"
#include <directfb.h>

extern IDirectFB *dfb;

struct callbackContext {
	lua_State *L;
	int index;
};

extern void init_directfb(lua_State *, int *, char ***);

extern void _include_SelSurface( lua_State * );
extern void _include_SelScreen( lua_State * );
extern void _include_SelLayer( lua_State * );
extern void _include_SelWindow( lua_State * );
extern void _include_SelImage( lua_State * );

extern int CooperativeConst( lua_State * );
extern int SetCooperativeLevel( lua_State * );

extern IDirectFBSurface **checkSelSurface(lua_State *, int );
#	endif

#endif
