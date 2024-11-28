/* Curse window storage
 */

#ifndef SELCURSESTORAGE_H
#define SELCURSESTORAGE_H

#include <Selene/libSelene.h>

#include <ncurses.h>

struct SelCurseStorage {
	struct ExportedSurface obj;	/* Object management */

	WINDOW *window;
};

#endif
