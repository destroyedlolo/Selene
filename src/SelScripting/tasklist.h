/* tasklist.h
 *
 * task list management
 *
 * Have a look and respect Selene Licence.
 */

#ifndef TASKLIST_H
#define TASKLIST_H

#include <Selene/SelScripting.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

#include <sys/eventfd.h>

#define FUNCREFLOOKTBL "__SELENE_FUNCREF"	/* Function reference lookup table */

extern struct SelScripting ss_selScripting;

extern struct SelLua *ss_selLua;
extern lua_State *ss_mainL;
extern struct SeleneCore *ss_selCore;
extern struct SelLog *ss_selLog;
extern int tlfd;

extern int ssc_pushtask( int, enum TaskOnce );
extern int ssc_handleToDoList(lua_State *);

extern int ssc_findFuncRef(lua_State *, int);
extern int ssl_registerfunc(lua_State *);
extern int ssl_TaskOnceConst(lua_State *);
extern int ssl_PushTaskByRef(lua_State *);
extern int ssl_PushTask(lua_State *);
extern bool ssc_isToDoListEmpty();
extern int ssl_dumpToDoList();

extern void ssc_AddStartupFunc(void (*)(lua_State *));
extern void ssc_ApplyStartupFunc(lua_State *);
#endif
