/* SelCurses.h
 *
 * Semi-graphical framework for textual display
 *
 *	Depends on industry standard libcurses
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELCURSES_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELCURSES_VERSION 1

struct SelCurses {
	struct SelModule module;

		/* Call backs */
};

#endif
