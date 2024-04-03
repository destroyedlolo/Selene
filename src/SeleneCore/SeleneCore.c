/* SeleneCore.c
 *
 * Selene's core and helpers
 *
 * 06/01/2024 First version
 */

#include <Selene/SeleneCore.h>
#include <Selene/SeleneVersion.h>

#include <stddef.h>		/* NULL */
#include <dlfcn.h>		/* dlopen(), ... */
#include <string.h>

static struct SeleneCore selCore;

static struct SelLog *selLog;

static bool scc_SelLogInitialised(struct SelLog *aselLog){
/**
 * @brief SelLog has been initialized.
 *
 * Initialise internal SelLog reference. After this call, SelCore's can
 * log messages.
 *
 * @function scc_SelLogInitialised
 * @param pointer to SelLog module
 * @return false if SelLog's is too old
 */
	selLog = aselLog;

	return(selLog->module.version >= SELLOG_VERSION);
}

static struct SelModule *scc_loadModule(const char *name, uint16_t minversion, uint16_t *verfound, char error_level){
/**
 * @brief Load a module
 *
 * @function loadModule
 * @param name Name of the module to load
 * @param minversion minimum version to load
 * @param found version of the found library (0 if not found, use dlerror() for explanation)
 * @param Error level to use in case of issue
 * @return pointer to the module or NULL if not found
 */
	struct SelModule *res = loadModule(name, minversion, verfound);

	if(selLog){
		if(!res){	/* An error occurred */
			if(*verfound)
				selLog->Log(error_level, "Can't load %s (%u instead of expected %u)",
					name, *verfound, minversion
				);
			else {
				char *err = dlerror();
				if(!err)
					selLog->Log(error_level, "Can't load %s : missing InitModule() or outdated dependency found", name);
				else
					selLog->Log(error_level, "Can't load %s (%s)", name, err);
			}
		} else
			selLog->Log('D', "%s found : version %u", name, *verfound);
	}

	return res;
}

static struct SelModule *scc_findModuleByName(const char *name, uint16_t minversion, char error_level){
/**
 * @brief Find a loaded module
 *
 * @function findModuleByName
 * @param name Name of the module
 * @param minversion minimum version
 * @param Error level to use in case of issue (0 to be quiet if not found)
 * @return pointer to the module or NULL if not found
 */
	struct SelModule *res = findModuleByName(name, 0);

	if(selLog){
		if(!res){
			if(error_level)
				selLog->Log(error_level, "Can't find %s", name);
			return NULL;
		} else if(res->version < minversion){
			if(error_level)
				selLog->Log(error_level, "Obsolete %s : %u instead of expected %u", 
					name, res->version, minversion
				);
			return NULL;
		}
	} else if(res->version < minversion)
		return NULL;

	return res;
}

static float scc_getVersion(){
/**
 * @brief Returns Selene's version
 *
 * @function getVersion
 * @return Number version number
 */
	return SELENE_VERSION;
}

static const int scc_findconst(const char *name, const struct ConstTranscode *tbl, bool *found){
/**
 * @brief find constant from its name
 * @tparam string name
 * @tparam ConstTranscode Table
 * @tparam bool found true if found
 * @treturn integer value (-1 if not found, and found == false)
 */
	*found = true;
	for(int i=0; tbl[i].name; i++){
		if(!strcmp(name, tbl[i].name))
			return tbl[i].value;
	}

	*found = false;
	return -1;
}

static const char *scc_rfindconst(const int id, const struct ConstTranscode *tbl){
/**
 * @brief find constant's name  from its value
 * @tparam integer value
 * @tparam ConstTranscode Table
 * @treturn string name (NULL if not found)
 */
	for(int i=0; tbl[i].name; i++)
		if(tbl[i].value == id)
			return tbl[i].name;

	return NULL;
}

static const char *scc_ctime(const time_t *t, char *s, size_t size){
/**
 * @brief like C ctime() but without leading carriage return
 * @tparam char * pointer to a buffer (if NULL, return a static buffer)
 * @tparam size_t buffer size
 * @return static string
 */

	if(!s){
		static char buff[26];
		s = buff;
		size = 26;
	}

	strftime(s, size, "%a %b %d %H:%M:%S", localtime(t));

	return s;
}

static bool scc_registerObject(struct SelModule *mod, struct _SelObject *obj, const char *name){
/**
 * @brief Initialize a module sub object
 *
 * @function scc_initObject
 * @param pointer to owner module
 * @param the object to be initialized
 * @param object's name
 * @return false if the object already exists
 */
	unsigned int H = selL_hash(name);
	struct _SelObject *t = selCore.findObject(mod, name, H);
	if(t)
		return false;

	obj->next = mod->objects;
	mod->objects = obj;
	obj->id.name = name;
	obj->id.H = H;

	return true;
}

static struct _SelObject *scc_findObject(struct SelModule *mod, const char *name, unsigned int H){
	struct _SelObject *obj = mod->objects;

	while(obj){
		if(obj->id.H == H && !strcmp(name, obj->id.name))
			return obj;
	}
	return NULL;
}

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){
	selLog = NULL;

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selCore, "SeleneCore", SELENECORE_VERSION, LIBSELENE_VERSION))
		return false;

	selCore.SelLogInitialised = scc_SelLogInitialised;
	selCore.loadModule = scc_loadModule;
	selCore.findModuleByName = scc_findModuleByName;
	selCore.getVersion = scc_getVersion;

	selCore.findConst = scc_findconst;
	selCore.rfindConst = scc_rfindconst;
	selCore.ctime = scc_ctime;

	selCore.registerObject = scc_registerObject;
	selCore.findObject = scc_findObject;

	registerModule((struct SelModule *)&selCore);

	return true;
}
