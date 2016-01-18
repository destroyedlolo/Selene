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
	int ifunc;		/* Function called "immediately" when timer expires */
	int task;		/* Function to put in the todo list when the timer expires */
	int once;		/* Avoid duplicate in the todo list ? */

			/*
			 * Fields used for Reset function
			 */
	lua_Number when;	/* Initial timer value */
	lua_Number rep;		/* Initial repeat interval */
};

#endif
