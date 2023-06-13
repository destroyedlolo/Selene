/* module.h
 * 	module managements
 *
 * This file is part of Selene project and is following the same
 * license rules (see LICENSE file)
 *
 * 	13/06/2023 - LF - First version (from Marcel project)
 */

#ifndef MODULE_H
#define MODULE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

	/* Modules are stored in a fixed length table. Their indexes are used
	 * to reference them is sections (which module/method is handling this section)
	 *
	 * This value may have to be increased when new modules are added ...
	 * up to 255. 
	 */
#define MAX_MODULES	16

	/* Module definition */
struct Module {
	const char *name;		/* module's name */
	int h;					/* hash code for the name */
	uint8_t module_index;

};

	/* list of loaded modules */
extern uint8_t number_of_loaded_modules;
extern struct Module *modules[];

	/* Management functions */
extern uint8_t findModuleByName(const char *name);
extern void register_module( struct Module * );

#ifdef __cplusplus
}
#endif

#endif
