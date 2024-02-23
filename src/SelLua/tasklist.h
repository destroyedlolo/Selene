/* tasklist.h
 *
 * task list management
 *
 * Have a look and respect Selene Licence.
 */

#ifndef TASKLIST_H
#define TASKLIST_H

#include <Selene/SelLua.h>
#include <Selene/SeleneVersion.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <sys/eventfd.h>

#define FUNCREFLOOKTBL "__SELENE_FUNCREF"	/* Function reference lookup table */

extern struct SelLua sl_selLua;
extern lua_State *sl_mainL;
extern struct SeleneCore *sl_selCore;
extern struct SelLog *sl_selLog;
extern int tlfd;

extern int slc_pushtask( int, enum TaskOnce );
extern int slc_handleToDoList(lua_State *);

extern int sll_registerfunc(lua_State *);
extern int sll_TaskOnceConst(lua_State *);
extern int sll_PushTaskByRef(lua_State *);
extern int sll_PushTask(lua_State *);
extern bool slc_isToDoListEmpty();
extern int sll_dumpToDoList();

#endif
