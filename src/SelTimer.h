/* SelTimer.h
 *
 * Everything to be shared for timers
 *
 * 03/07/2015 LF : First version
 * 20/01/2016 LF : rename as SelTimer.c
 */

#ifndef SELTIMER_H
#define SELTIMER_H

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

	/* Shared functions */
void init_SelTimer( lua_State * );
const char *_TimerReset( struct SelTimer * );
#endif