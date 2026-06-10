/***
 * Surface defintions shared by SelLCDSurface and SelLCDsubSurface.
 */

#ifndef SELLCDSHAREDSURFACE_H
#define SELLCDSHAREDSURFACE_H

#include <Selene/libSelene.h>
#include <Selene/SelGenericSurface.h>
#include <Selene/SelPlug-in/SelLCD/SelLCD.h>

struct SelLCDCoordinate {
	uint8_t x,y;
};

struct RestrictArea {
	uint8_t x,y;	/* Origine */
	uint8_t w,h;	/* size */
};

struct SelLCDSharedSurface {
	struct SelGenericSurface obj;	/* Object management */

	struct SelLCDSharedSurface *parent;
	struct SelLCDScreen *screen;
	uint8_t w,h;				/* Size of the surface */
	struct SelLCDCoordinate origine;	/* It's top left corner (absolute to the screen) */
	struct SelLCDCoordinate cursor;	/* Cursor position (relative) */
};

struct SelLCDSharedSurfaceLua {
	struct SelLCDSharedSurface *storage;
};

extern void *slss_getPrimary(struct SelLCDSharedSurface *);
extern bool slss_inSurface(struct SelLCDSharedSurface *lcd, uint32_t x, uint32_t y);	/* x,y relative to the surface */
#endif
