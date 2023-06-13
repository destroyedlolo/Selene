/* module.c
 * 	module managements
 *
 * This file is part of Selene project and is following the same
 * license rules (see LICENSE file)
 *
 * 	13/06/2023 - LF - First version (from Marcel project)
 */

#include "Module.h"
#include "libSelene.h"

#include <string.h>
#include <stdlib.h>

uint8_t number_of_loaded_modules = 0;
struct Module *modules[MAX_MODULES];

/**
 * @brief Search for a section
 *
 * @param name Name of the section we are looking for
 * @return uid of this module (-1 if not found)
 */
uint8_t findModuleByName(const char *name){
	int h = SelL_hash(name);

	for(int i = 0; i < number_of_loaded_modules; i++){
		if(modules[i]->h == h && !strcmp(name, modules[i]->name))
			return i;
	}

	return (uint8_t)-1;	/* Not found */
}

/**
 * @brief Register a module.
 * Add it in the liste of known modules
 */
void register_module( struct Module *mod ){
	if(findModuleByName(mod->name) != (uint8_t)-1){
		slc_log('F', "Module is already loaded");
		exit(EXIT_FAILURE);
	}

	if(number_of_loaded_modules >= MAX_MODULES){
		slc_log('F',"Too many registered modules. Increase MAX_MODULES");
		exit( EXIT_FAILURE );
	}

	mod->module_index = number_of_loaded_modules;
	mod->h = SelL_hash(mod->name);
	modules[number_of_loaded_modules++] = mod;
}
