/* Definitions shared among LCD sources */

#ifndef SELLCDSHARED_H
#define SELLCDSHARED_H

#include <Selene/SelPlug-in/SelLCD/SelLCDSharedSurface.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

extern struct SelLCD slcd_selLCD;		/* Module */

	/* Lua methods table export shared among all objects */
extern const struct luaL_Reg LCDShared[];

extern struct SeleneCore *slcd_selCore;
extern struct SelLog *slcd_selLog;
extern struct SelLua *slcd_selLua;

extern void initExportedSurface(struct SelLCDSharedSurface *, struct SelLCDSharedSurface *parent, uint8_t width, uint8_t height, uint8_t left, uint8_t top, struct SelLCDScreen *);

	/* Screen's */
extern const struct luaL_Reg LCDScreenMethods[];
extern void initSLScreenCallBacks();		/* init surface callbacks */

#endif
