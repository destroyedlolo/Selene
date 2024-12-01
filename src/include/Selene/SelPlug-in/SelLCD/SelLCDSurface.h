/***
 * Surface on textual LCD screen
 */

#ifndef SELLCDSURFACE_H
#define SELLCDSURFACE_H

#include <Selene/libSelene.h>
#include <Selene/SelPlug-in/SelLCD/SelLCD.h>

typedef struct SelLCDCoordinate {
	uint8_t x,y;
} SelLCDCoordinate;

struct SelLCDSurface {
	struct ExportedSurface obj;	/* Object management */

	struct SelLCDSurface *parent;
	uint8_t w,h;				// Size of the surface
	SelLCDCoordinate origine;	// It's top left corner
};
#endif
