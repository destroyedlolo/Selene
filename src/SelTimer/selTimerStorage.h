/* selTimerStorage.h
 *
 * SelTimer's own data
 */
#ifndef SELTIMERSTORAGE_H
#define SELTIMERSTORAGE_H
#include <stdbool.h>

struct selTimerStorage {
	int fd;			/* File descriptor for this timer */
	int ifunc;		/* Function called "immediately" when timer expires */
	int task;		/* Function to put in the todo list when the timer expires */
	bool once;		/* Avoid duplicate in the todo list ? */
	bool disable;	/* if set, tasks are not launched */

			/*
			 * Fields used for Reset function
			 */
	lua_Number when;	/* Initial timer value */
	lua_Number rep;		/* Initial repeat interval */
};
#endif
