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

/*
 * can be overwritten from command line if needed 
 */

extern struct SelLua sl_selLua;
extern lua_State *sl_mainL;
extern struct SeleneCore *sl_selCore;
extern struct SelLog *sl_selLog;
extern int tlfd;

extern int slc_pushtask( int, enum TaskOnce );

#endif
