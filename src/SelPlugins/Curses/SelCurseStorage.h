/* Curse window storage
 */

#ifndef SELCURSESTORAGE_H
#define SELCURSESTORAGE_H

#include <Selene/libSelene.h>
#include <Selene/SelGenericSurface.h>

#include <ncurses.h>

struct SelCurseStorage {
	struct SelGenericSurface obj;	/* Object management */

	WINDOW *window;
};

extern bool initExportedWindow(struct SelCurseStorage *, WINDOW *);
#endif
