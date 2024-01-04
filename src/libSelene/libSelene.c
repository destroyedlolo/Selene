/* libSelene.h
 *
 * 04/01/2024 First version
 */

#include "libSelene.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>	/* dl*() */

struct SelModule *modules = NULL;

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
 * @brief Load a module
 *
 * @param name Name of the module to load
 * @param minversion minimum version to load
 * @param found version of the found library (0 if not found, use dlerror() for explanation)
 * @return pointer to the module or NULL if not found
 */
struct SelModule *loadModule(const char *name, uint16_t minversion, uint16_t *found){
	*found = 0;
	dlerror(); /* Clear any existing error */

		/* check if it is already loaded */
	struct SelModule *res = findModuleByName(name);
	if(res){
		*found = res->version;
		if(res->version >= minversion)
			return res;		/* ok, found it */
		else 
			return NULL;	/* obsolete version loaded */
	}

		/* Load from disk */
	char t[strlen(PLUGIN_DIR) + strlen(name) + 5];	/* "/.so" */
	sprintf(t, "%s/%s.so", PLUGIN_DIR, name);

	void *pgh = dlopen(t, RTLD_LAZY);
	if(!pgh)
		return NULL;	/* .so not found */
	dlerror(); /* Clear any existing error */

		/* execute, initialisation function */
	void (*func)(void);
	if(!(func = dlsym( pgh, "InitModule" ))){
		fputs("*F* Can't find InitModule()\n", stderr);
		exit(EXIT_FAILURE);
	}

		/* Execute it.
		 * It has to register the newly loaded module. Consequently, it is the
		 * 1st of modules' list.
		 */
	(*func)();

	return modules;
}

/**
 * @brief Search for a loaded module.
 *
 * @param name Name of the module we are looking for
 * @return pointer to the module or NULL if not found
 */
struct SelModule *findModuleByName(const char *name){
	unsigned int h = selL_hash(name);

	for(struct SelModule *res = modules; res; res = res->next){
		if(res->h == h && !strcmp(name, res->name))
			return res;
	}

	return NULL;
}
