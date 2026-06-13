/***
 * LCD SubSurface
 *
 *	SubSurfaces are only restricted area of a main surface 
 *	(SelSurface or SelScreen)
 */

#ifndef SELLCDSUBSURFACE
#define SELLCDSUBSURFACE

#include <Selene/SelPlug-in/SelLCD/SelLCDSharedSurface.h>

struct SelLCDSubSurface {
	struct SelLCDSharedSurface shared;

		/* For the moment, nothing more.
		 * But a dedicated structure is created for easying futures evolutions
		 */
};

struct SelLCDSubSurfaceLua {
	struct SelLCDSubSurface *storage;
};

#endif
