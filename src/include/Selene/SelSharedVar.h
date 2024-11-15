/* SelSharedVar.h
 *
 * Variable shared among threads
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELSHAREDVAR_VERSION

#include <Selene/libSelene.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELSHAREDVAR_VERSION 1

	/* ***
	 * Shared variables
	 * ***/

enum SharedObjType {
	SOT_UNKNOWN = 0,	/* Invalid variable */
	SOT_NUMBER,		/* Integers */
	SOT_STRING,		/* Dynamically allocated string (managed by sharedobj' functions) */
	SOT_XSTRING		/* Const char * managed externally (constant, allocated elsewhere ...) */
};

union SelSharedVarContent {
	double num;
	const char *str;
};

struct SelSharedVar {
	struct SelModule module;

		/* Call backs */
	void (*clear)(const char *);
	void (*setNumber)(const char *, double, unsigned long int);
	void (*setString)(const char *, const char *, unsigned long int);

	enum SharedObjType (*getType)(const char *);
	union SelSharedVarContent (*getValue)(const char *, enum SharedObjType *, bool);
	void (*unlockVariable)(const char *);
};

#endif
