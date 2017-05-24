/* SelEvent.h
 *
 * Everything to be shared for events
 *
 * 24/03/2017 LF : First version
 */

#ifndef SELEVENT_H
#define SELEVENT_H

struct SelEvent {
	int fd;		/* File descriptor */
	int func;	/* Function to be called */
};

	/* Shared functions */
void init_SelEvent( lua_State * );

#endif
