/* selTimer.h
 *
 * This file contains definitions shared among several source files
 */
#ifndef SELTIMER_H
#define SELTIMER_H

struct selTimerStorage {
	int fd;			/* File descriptor for this timer */
	int ifunc;		/* Function called "immediately" when timer expires */
	int task;		/* Function to put in the todo list when the timer expires */
	int once;		/* Avoid duplicate in the todo list ? */
	int disable;	/* if set, tasks are not launched */

			/*
			 * Fields used for Reset function
			 */
	lua_Number when;	/* Initial timer value */
	lua_Number rep;		/* Initial repeat interval */
};
#endif
