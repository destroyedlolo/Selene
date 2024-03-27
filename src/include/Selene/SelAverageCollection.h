/* SelAverageCollection.h
 *
 * This collection stores **immediate** and **average** values sub collection.
 *
 * 11/06/2022 LF : creation
 * 27/03/2024 LF : migrate to v7
 */

#ifndef SELAVERAGECOLLECTION_VERSION

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELAVERAGECOLLECTION_VERSION 1

struct SelAverageCollectionStorage;

struct SelAverageCollection {
	struct SelModule module;

		/* Call backs */
	struct SelAverageCollectionStorage *(*create)(size_t, size_t, size_t, size_t);
};

#endif
