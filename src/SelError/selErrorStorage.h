/* selErrorStorage.h
 *
 * SelError's own data
 */
#ifndef SELERRORSTORAGE_H
#define SELERRORSTORAGE_H

struct selErrorStorage {
	struct SelObject obj;	/* Object management */

	char level;			/* Error level */
	const char *msg;	/* Error message */
};

#endif
