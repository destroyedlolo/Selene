/* SelTimer.h
 *
 * Multi purposes and versatile timer
 *
 * Have a look and respect Selene Licence.
 */
#ifndef SELTIMER_VERSION

#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELTIMER_VERSION 2

#ifdef __cplusplus
extern "C"
{
#endif

struct selTimerStorage;

struct SelTimer {
	struct SelModule module;

		/* Call backs */
	const char *(*reset)(struct selTimerStorage *);
	int (*getFD)(void *);
	int (*getiFunc)(void *);
	int (*getTask)(void *);
	bool (*getOnce)(void *);
	bool (*isDisabled)(void *);
	struct selTimerStorage *(*find)(const char *, unsigned int);
};

#ifdef __cplusplus
}
#endif

#endif
