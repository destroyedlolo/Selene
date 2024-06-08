/* SelSharedRefStorage.h
 *
 * Shared Reference
 */

#ifndef SELSHAREDREFSTORAGE_H
#define SELSHAREDREFSTORAGE_H

#include <Selene/SelSharedRef.h>

struct SelSharedRefStorage {
	struct _SelObject obj;	/* Object management */

	int ref;
};

#endif