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

extern void initSharedSurface(struct SelLCDSharedSurface *, struct SelLCDSharedSurface *parent, uint8_t width, uint8_t height, uint8_t left, uint8_t top, struct SelLCDScreen *);

	/* Screen's */
extern const struct luaL_Reg LCDScreenMethods[];
extern void initSLScreenCallBacks();		/* init surface callbacks */
extern void initSelLCDScreen(struct SelLCDScreen *);	/* initialize structure */

	/* SubSurface's */
extern struct SGS_callbacks cb_subsurface;
extern const struct luaL_Reg LCDSubSurfaceMethods[];
extern void initSLLCDSubSurfaceCallBacks();		/* init subsurface callbacks */

	/* Surface's */
extern struct SGS_callbacks cb_surface;
extern const struct luaL_Reg LCDSurfaceMethods[];
extern void initSLLCDSurfaceCallBacks();		/* init surface callbacks */
#endif
