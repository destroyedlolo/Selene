/* SelSharedFunctionStorage.h
 *
 * Shared function
 */

#ifndef SELSHAREDFUNCTIONSTORAGE_H
#define SELSHAREDFUNCTIONSTORAGE_H

#include <Selene/SelSharedFunction.h>

struct SelSharedFunctionStorage {
	struct _SelObject obj;	/* Object management */

	struct elastic_storage estorage;
};

#endif
