/* SelTimedWindowCollection.h
 *
 * Collection of timed windowed values
 *
 */

#ifndef SELTIMEDWINDOWCOLLECTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELTIMEDWINDOWCOLLECTION_VERSION 1

#include <time.h>

struct SelTimedWindowCollectionStorage;

struct SelTimedWindowCollection {
	struct SelModule module;

};

#endif
