/* SelEvent.h
 *
 * Everything to be shared for events
 *
 * 24/03/2017 LF : First version
 * 07/04/2018	LF : Migrate to Selene v4
 */

#ifndef SELEVENT_H
#define SELEVENT_H

struct SelEvent {
	int fd;		/* File descriptor */
	int func;	/* Function to be called */
};

#endif
