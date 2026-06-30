/***
 * Surface defintions shared by SelLCDSurface and SelLCDsubSurface.
 */

#ifndef SELLCDSHAREDSURFACE_H
#define SELLCDSHAREDSURFACE_H

#include <Selene/libSelene.h>
#include <Selene/SelGenericSurface.h>
#include <Selene/SelPlug-in/SelLCD/SelLCD.h>

struct SelCoordinate {
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
	struct SelCoordinate origine;	/* It's top left corner (absolute to the screen) */
	struct SelCoordinate cursor;	/* Cursor position (relative) */
};

struct SelLCDSharedSurfaceLua {
	struct SelLCDSharedSurface *storage;
};

extern bool lcdss_getSize(struct SelLCDSharedSurface *, uint32_t *, uint32_t *);
extern void *lcdss_getPrimary(struct SelLCDSharedSurface *);
extern void *lcdss_getParent(struct SelLCDSharedSurface *);
extern struct SelLCDSubSurface *lcdss_subSurface(struct SelLCDSharedSurface *p, uint32_t x, uint32_t y, uint32_t w, uint32_t h, struct SelLCDScreen *lcd);
extern struct SelLCDSurface *lcdss_Surface(struct SelLCDSharedSurface *p, uint32_t x, uint32_t y, uint32_t w, uint32_t h, struct SelLCDScreen *lcd);
extern bool lcdss_Home(struct SelLCDSharedSurface *);
extern bool lcdss_setCursor(struct SelLCDSharedSurface *, uint32_t, uint32_t );
extern bool lcdss_inSurface(struct SelLCDSharedSurface *lcd, uint32_t x, uint32_t y);	/* x,y relative to the surface */
extern bool lcdss_Clear(struct SelLCDSharedSurface *);
extern bool lcdss_WriteString(struct SelLCDSharedSurface *, const char *);
#endif
