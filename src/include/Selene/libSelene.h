/* libSelene.h
 *
 * This library contains the strict minimum code to load dynamic modules.
 * The goal is to avoid tight linkage within application binary in order to
 * avoid recompilation when something changes within Selene like adding
 * functions. This method is similar to the way AmigaOS handles its own
 * libraries.
 *
 * Golden rules :
 * - enforce strict upward compatibility
 * - bump version at every interface change.
 *
 * Have a look and respect Selene Licence.
 */

#ifndef LIBSELENE_VERSION

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define LIBSELENE_VERSION 2

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/* *****
 * Modules glue structure
 *
 * This structure must stay as simple as possible : it's the only one
 * which is tightly linked with Selene's clients. In other words, if it's
 * changed, it will require clients to be recompiled.
 */

struct SelModule {
	struct SelModule *next;	/* Pointer to next module */
	uint16_t SelModVersion;	/* version of SelModule structure */

	const char *name;		/* module's name */
	unsigned int h;			/* hash code for the name */
	uint16_t version;		/* Module version */
};

	/* list of loaded modules */
extern struct SelModule *modules;

	/* Management functions */
extern unsigned int selL_hash(const char *);
extern struct SelModule *loadModule(const char *name, uint16_t minversion, uint16_t *);
extern struct SelModule *findModuleByName(const char *name, uint16_t version);

	/* Initialisation function */
extern bool initModule(struct SelModule *, const char *name, uint16_t version, uint16_t libSelene_version);
extern bool registerModule(struct SelModule *);

#ifdef __cplusplus
}
#endif

#endif
