/* directfb.h
 *
 * All shared definitions related to directFB
 *
 * 13/04/2015 LF : First version
 */

#ifndef SDIRECTFB_H
#define SDIRECTFB_H

#	ifdef USE_DIRECTFB
#include "selene.h"
#include <directfb.h>

extern void init_directfb(lua_State *, int *, char ***);

extern int CooperativeConst( lua_State * );
extern int CapabilityConst( lua_State * );
extern int TextLayoutConst( lua_State * );

extern int SetCooperativeLevel( lua_State * );

#	endif

#endif
