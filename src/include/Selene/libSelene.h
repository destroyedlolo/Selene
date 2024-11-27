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
#define LIBSELENE_VERSION 9

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

/* ***
 * store names of objects
 * ***/

struct NameH {
	const char *name;
	int H;	/* Hash code for this name */
};

/* ***
 * Shared named object fields
 * Used to find collection by name
 */

struct _SelNamedObject {
	struct _SelNamedObject *next;
	struct NameH id;
};

/* *****
 * Modules glue structure
 *
 * This structure must stay as simple as possible : it's the only one
 * which is tightly linked with Selene's clients. In other words, if it's
 * changed, it will require clients to be recompiled.
 */

struct SelLua;

/* ****
 * Capabilities
 */

	/* Kind of module */
#define SELCAP_UI 0x01		/* User interface : can be used to display something */

	/* UI specifics 
	 * Example :
	 * 	LCD 1602 : SELCAP_UI
	 * 		- A simple monochrom textual display
	 * 	CURSE : SELCAP_UI | SELCAPUI_GFX | SELCAPUI_COLOR
	 * 		- despite text based, some graphics can be simulated
	 * 		- color can be changed as well : N&B or color depending
	 * 		on terminal capabilities.
	 * 	OLED : SELCAP_UI | SELCAPUI_GFX | SELCAPUI_COLOR
	 * 		- graphical display where color can be set (N&B)
	 * 	HDMI : SELCAP_UI | SELCAPUI_GFX | SELCAPUI_COLOR
	 * 		- graphical display where color can be set (true colors)
	 */
#define SELCAPUI_GFX	0x10000	/* Can display graphics (textual otherwise) */
#define SELCAPUI_COLOR	0x20000	/* color can be set */

struct SelModule {
	struct SelModule *next;	/* Pointer to next module */
	uint16_t SelModVersion;	/* version of SelModule structure */

	struct NameH name;
	uint16_t version;		/* Module version */
	bool found;

	struct _SelNamedObject *objects;			/* Objects related to this module */
	pthread_mutex_t nobjmutex;	/* protect concurrent access to objects list*/

		/* Call backs */
	bool (*initLua)(struct SelLua *);		/* Lua initialisation function (for modules loaded before SelLua and one loaded by Selene.Use())*/
	bool (*checkdependencies)();	/* returns if all dependencies are met */
	void (*dump)();		/* Logs module's status */
	bool (*laterebuilddependancies)();	/* rebuild missing dependencies */

	void (*exposeAdminAPI)(void *); /* Request to expose administration API. Real arg is lua_State * */

	uint64_t (*getCapabilities)();		/* Returns all capabilities */
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

	/* Capabilities */
extern bool checkCapabilities(struct SelModule *, uint64_t);
#ifdef __cplusplus
}
#endif

#endif
