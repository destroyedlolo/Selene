/***
 * LCD screen impersonation
 */

#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDScreen.h>

struct SGS_callbacks cb_screen;

static const char * const LuaName(){
	return "SelLCDScreen";
}

static void *slscrn_getPrimary(struct SelLCDSharedSurface *s){
	/* Returns the primary */
	return s->screen;
}

void initSLScreenCallBacks(){
	cb_screen.LuaObjectName = LuaName;

		/* It's the physical screen, so mostly wrappers to the module */
	cb_screen.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))slcd_selLCD.GetSize;
	cb_screen.Home = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Home;
/*	cb_screen.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))slcd_.subSurface;*/
	cb_screen.getPrimary = (void *(*)(struct SelGenericSurface *))slscrn_getPrimary;
}
