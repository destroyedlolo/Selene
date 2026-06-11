/***
 * LCD screen impersonation
 */

#include "SelLCDShared.h"
#include <Selene/SelPlug-in/SelLCD/SelLCDScreen.h>

#include <errno.h>	/* EBUSY */
#include <stdlib.h>	/* malloc() */
#include <assert.h>

struct SGS_callbacks cb_screen;

static const char * const LuaName(){
	return "SelLCDScreen";
}

static bool slss_Lock(struct SelLCDScreen *scr, bool heavy){
		/* Try to lock the mutex */
	if(pthread_mutex_trylock(&scr->mutex) == EBUSY){
			/* Wait to get the lock */
		pthread_mutex_lock(&scr->mutex);

			/* Lets the previous operation (the only that acquired
			 * the lock), to finish
			 */
		if(heavy)
			usleep(1520);	/* 1.52 ms */
		else
			usleep(37);		/* 37 us */
	}

	return true;
}

static bool slss_Unlock(struct SelLCDScreen *scr){
	pthread_mutex_unlock(&scr->mutex);
	return true;
}

bool slss_AllocBuff(struct SelLCDScreen *scr){
#ifdef DEBUG
	slcd_selLog->Log('D', "Buffers allocation");
#endif

		/* Free existing buffers */
	if(scr->working_buffer){
		free(scr->working_buffer);
		scr->working_buffer = NULL;	/* Only if there is a failure afterward */
	}
	if(scr->screen_buffer){
		free(scr->screen_buffer);
		scr->screen_buffer = NULL;	/* Only if there is a failure afterward */
	}

	scr->working_buffer = malloc(scr->primary.w * scr->primary.h);
	assert(scr->working_buffer);
	scr->screen_buffer =  malloc(scr->primary.w * scr->primary.h);
	assert(scr->screen_buffer);

	return(scr->working_buffer && scr->screen_buffer);
}

void initSLScreenCallBacks(){
	cb_screen.LuaObjectName = LuaName;

		/* It's the physical screen, so mostly wrappers to the module */
	cb_screen.getSize = (bool (*)(struct SelGenericSurface *, uint32_t *, uint32_t *))slcd_selLCD.GetSize;
	cb_screen.Home = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Home;
/*	cb_screen.subSurface = (struct SelGenericSurface *(*)(struct SelGenericSurface *, uint32_t,  uint32_t,  uint32_t,  uint32_t, void *))slcd_.subSurface;*/
	cb_screen.getPrimary = (void *(*)(struct SelGenericSurface *))slss_getPrimary;

	cb_screen.setCursor = (bool (*)(struct SelGenericSurface *, uint32_t, uint32_t))slcd_selLCD.SetCursor;
	cb_screen.inSurface = (bool (*)(struct SelGenericSurface *, uint32_t,  uint32_t))slss_inSurface;
	cb_screen.Clear = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Clear;
	cb_screen.WriteString = (bool (*)(struct SelGenericSurface *, const char *))slcd_selLCD.WriteString;

	cb_screen.Lock = (bool (*)(struct SelGenericSurface *, bool))slss_Lock;		/* Lock on physical screen */
	cb_screen.Unlock = (bool (*)(struct SelGenericSurface *))slss_Unlock;	/* Lock on physical screen */

	cb_screen.AllocateBuffer = (bool (*)(struct SelGenericSurface *))slss_AllocBuff;
	cb_screen.Refresh = (bool (*)(struct SelGenericSurface *))slcd_selLCD.Refresh;
	cb_screen.Dump = (bool (*)(struct SelGenericSurface *))slcd_selLCD.DumpBuffers;
}
