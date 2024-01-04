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

#ifndef LIBSELENE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define LIBSELENE_H	1

#include <stdbool.h>
#include <stdint.h>

/* *****
 * Modules definition
 *
 * Modules are dynamicaly loaded using LoadModule() function.
 *
 * Modules are stored in a fixed length table. Their indexes are used
 * to reference them is sections (which module/method is handling this
 * section)
 *
 * This value may have to be increased when new modules are added ...
 * up to 255. 
 */
#define MAX_MODULES	16

struct SelModule {
	struct SelModule *next;	/* Pointer to next module */

	const char *name;		/* module's name */
	unsigned int h;			/* hash code for the name */
	uint16_t version;		/* Module version */
};

	/* list of loaded modules */
extern struct SelModule *modules;

	/* Management functions */
extern unsigned int selL_hash(const char *);
extern struct SelModule *loadModule(const char *name, uint16_t minversion, uint16_t *);
extern struct SelModule *findModuleByName(const char *name);
extern void initModule(struct SelModule *, const char *name, uint16_t version);
extern void register_module(struct SelModule *);

#ifdef __cplusplus
}
#endif

#endif
