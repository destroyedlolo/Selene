/***
 * Surface defintions shared by SelLCDSurface and SelLCDsubSurface.
 */

#ifndef SELLCDSHAREDSURFACE_H
#define SELLCDSHAREDSURFACE_H

#include <Selene/libSelene.h>
#include <Selene/SelGenericSurface.h>
#include <Selene/SelPlug-in/SelLCD/SelLCD.h>

typedef struct SelLCDCoordinate {
	uint8_t x,y;
} SelLCDCoordinate;


struct RestrictArea {
	uint8_t x,y;	/* Origine */
	uint8_t w,h;	/* size */
};

struct SelLCDSharedSurface {
	struct SelGenericSurface obj;	/* Object management */

	struct SelLCDSharedSurface *parent;
	struct SelLCDScreen *screen;
	uint8_t w,h;				/* Size of the surface */
	SelLCDCoordinate origine;	/* It's top left corner (absolute to the screen) */
	SelLCDCoordinate cursor;	/* Cursor position (relative) */
};

struct SelLCDSharedSurfaceLua {
	struct SelLCDSharedSurface *storage;
};
#endif
