/* SelEventStorage.h
 *
 * SelEvent own storage
 */
#ifndef SELEVENTSTORAGE_H
#define SELEVENTSTORAGE_H

struct SelEventStorage {
	int fd;		/* File descriptor */
	int func;	/* Function to be called */
};

#endif
