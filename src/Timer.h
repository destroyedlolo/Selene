/* Timer.h
 *
 * Everything to be shared for timers
 *
 * 03/07/2015 LF : First version
 */

#ifndef SELTIMER_H
#define SELTIMER_H

void init_SelTimer( lua_State * );

struct SelTimer {
	int fd;			/* File descriptor for this timer */
	int trigger;	/* Function called when timer expires */
};

#endif
