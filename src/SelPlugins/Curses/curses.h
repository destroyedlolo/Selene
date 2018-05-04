/* curses.h
 *
 * All shared definitions related to curses
 *
 * 06/09/2016 LF : First version
 * 03/05/2018 LF : Switch to Selene v4
 */

#ifndef SCURSES_H
#	define SCURSES_H

#	ifdef USE_CURSES

#include "../../SeleneLibrary/libSelene.h"
#include <ncurses.h>

extern void initSelCurses(lua_State *);

extern void _include_SelCWindow( lua_State * );

#	endif
#endif
