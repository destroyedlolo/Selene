/***
 * Surface defintions shared by SelLCDSurface and SelLCDsubSurface.
 */

#include <Selene/SelPlug-in/SelLCD/SelLCDSharedSurface.h>

void *slss_getPrimary(struct SelLCDSharedSurface *s){
	return s->screen;
}

bool slss_inSurface(struct SelLCDSharedSurface *s, uint32_t x, uint32_t y){
	return( x < s->w && y < s->h );
}

