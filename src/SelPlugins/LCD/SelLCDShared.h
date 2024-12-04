/* Definitions shared among LCD sources */

#ifndef SELLCDSHARED_H
#define SELLCDSHARED_H

#include <Selene/SelPlug-in/SelLCD/SelLCDSurface.h>
#include <Selene/SeleneCore.h>
#include <Selene/SelLog.h>

extern struct SelLCD slcd_selLCD;
extern const struct luaL_Reg LCDSM[];

extern struct SeleneCore *slcd_selCore;
extern struct SelLog *slcd_selLog;
extern struct SelLua *slcd_selLua;

extern void initExportedSurface(struct SelLCDSurface *, struct SelLCDSurface *parent, uint8_t width, uint8_t height, uint8_t left, uint8_t top );

extern void initSLSCallBacks();
#endif
