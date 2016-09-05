/* curses.h
 *
 * All shared definitions related to curses
 *
 * 06/09/2016 LF : First version
 */

#ifndef SCURSES_H
#	define SCURSES_H

#	ifdef USE_CURSES

#define _POSIX_C_SOURCE 199309	/* Otherwise some defines/types are not defined with -std=c99 */

#include "../../selene.h"
#include <ncurses.h>

extern void init_curses(lua_State *);

#	endif
#endif
