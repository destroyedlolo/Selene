/* libSelene.h
 *
 * 04/01/2024 First version
 */

#include "libSelene.h"

#include <string.h>

uint8_t number_of_loaded_modules = 0;
struct SelModule *modules[MAX_MODULES];

/**
 * Calculate the hash code of the given string
 *
 * @param s string to calculate the hash code
 * @return hashcode
 */
unsigned int selL_hash(const char *s){
	unsigned int r = 0;
	for(; *s; s++)
		r += *s;
	return r;	
}

/**
 * @brief Search for a loaded module.
 *
 * @param name Name of the module we are looking for
 * @param minversion minimum version to load
 * @param loadedversion version of the found library (0 if not found)
 * @return uid of this module (-1 if not found)
 */
uint8_t findModuleByName(const char *name, uint16_t minversion, uint16_t *loadedversion){
	unsigned int h = selL_hash(name);

	*loadedversion = 0;
	for(int i = 0; i < number_of_loaded_modules; i++){
		if(modules[i]->h == h && !strcmp(name, modules[i]->name)){
			*loadedversion = modules[i]->version;
			if(modules[i]->version >= minversion)
				return i;	/* ok, matching */
			else
				return (uint8_t)-1;	/* Version doesn't match */
		}
	}

	return (uint8_t)-1;	/* Not found */
}


