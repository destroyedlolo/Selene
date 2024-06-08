/* libSelene.h
 *
 * 04/01/2024 First version
 */

#include <Selene/libSelene.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>	/* dl*() */

struct SelModule *modules = NULL;

/**
 * Calculate the hash code of the given string
 *
 * @function selL_hash
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
 * @function loadModule
 * @param name Name of the module to load
 * @param minversion minimum version to load
 * @param found version of the found library (0 if not found, use dlerror() for explanation)
 * @return pointer to the module or NULL if not found
 */
struct SelModule *loadModule(const char *name, uint16_t minversion, uint16_t *found){
	*found = 0;
	dlerror(); /* Clear any existing error */

		/* check if it is already loaded */
	struct SelModule *res = findModuleByName(name,0);
	if(res){
		*found = res->version;
		if(res->version >= minversion)
			return res;		/* ok, found it */
		else 
			return NULL;	/* obsolete version loaded */
	}

		/* Load from disk */
	char t[strlen(PLUGIN_DIR) + strlen(name) + 12];	/* "/Selene/.so" */
	sprintf(t, "%s/Selene/%s.so", PLUGIN_DIR, name);

	void *pgh = dlopen(t, RTLD_LAZY);
	if(!pgh)
		return NULL;	/* .so not found */
	dlerror(); /* Clear any existing error */

		/* execute, initialisation function */
	bool (*func)(void);
	if(!(func = dlsym( pgh, "InitModule" ))){
		fputs("*F* Can't find InitModule()\n", stderr);
		exit(EXIT_FAILURE);
	}

		/* Execute it.
		 * It has to register the newly loaded module. Consequently, it is the
		 * 1st of modules' list.
		 */
	if(!(*func)())
		return NULL;

	*found = modules->version;
	if(modules->version < minversion)	/* Obsolete version loaded */
		return NULL;

	return modules;
}

/**
 * @brief Search for a loaded module.
 *
 * @function findModuleByName
 * @param name Name of the module we are looking for
 * @param version minimum version to take in account
 * @return pointer to the module or NULL if not found
 */
struct SelModule *findModuleByName(const char *name, uint16_t version){
	unsigned int h = selL_hash(name);

	for(struct SelModule *res = modules; res; res = res->next){
		if(res->name.H == h && !strcmp(name, res->name.name)){
			if(res->version < version)
				return NULL;
			else
				return res;
		}
	}

	return NULL;
}

static bool truebydefault(){
	return true;
}

/**
 * @brief Initialise basic module structure
 *
 * @function initModule
 * @param module structure to initialise
 * @param name Name of the module
 * @param version version of the module
 * @param libSelene_version version of SelModule
 */
bool initModule(struct SelModule *module, const char *name, uint16_t version, uint16_t libSelene_version){
	if(libSelene_version > LIBSELENE_VERSION)	/* expecting newer version */
		return false;

	module->next = NULL;
	module->SelModVersion = libSelene_version;

	module->name.name = name;
	module->name.H = selL_hash(name);

	module->version = version;
	module->found = false;

	module->objects = NULL;

	module->initLua = NULL;
	module->checkdependencies = truebydefault;	/* by default, all dependencies are met */
	module->dump = NULL;
	module->laterebuilddependancies = NULL;

	/* Additional fields bellow came with extended SelModule structure
	 * and are based on its versions
	 */

	if(libSelene_version > 7){
		module->exposeAdminAPI = NULL;
	}

	return true;
}

/**
 * @brief Add module to in memory modules' list
 *
 * @function registerModule
 * @param module structure to register
 * @return does the registering succeed ?
 */
bool registerModule(struct SelModule *module){
	if(findModuleByName(module->name.name,0))
		return false;	/* Fail as a module with the same name exists */

	module->next = modules;
	modules = module;

	return true;	/* Success */
}
