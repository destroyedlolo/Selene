/* sharedobj.h
 *
 * Everything related to shared variables
 *
 * 07/06/2015 LF : First version
 */

#ifndef SHAREDOBJ_H
#define SHAREDOBJ_H

#include "selene.h"

void init_shared( lua_State * );
void init_shared_Lua( lua_State * );	/* Init only Lua's object */

	/* Tasks manipulation */
int pushtask( int );

#endif
