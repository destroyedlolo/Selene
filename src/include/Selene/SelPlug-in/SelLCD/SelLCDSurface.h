/***
 * LCD Surface
 *
 *	Surfaces is like a subSurface but with its dedicated buffer
 */

#ifndef SELLCDSURFACE
#define SELLCDSURFACE

#include <Selene/SelPlug-in/SelLCD/SelLCDSharedSurface.h>

struct SelLCDSurface{
	struct SelLCDSharedSurface shared;

	char *buffer;

	bool visible;
};

struct SelLCDSurfaceLua {
	struct SelLCDSurface *storage;
};

#endif
