/* SelGenericSurface.h
 *
 * Generic definitions of surfaces.
 * Shared among all renderer back end.
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELENEGENERICSURFACE_H

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELENEGENERICSURFACE_H

#include <Selene/libSelene.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ****
 * Capabilities
 */

	/* Kind of module */
#define SELCAP_RENDERER 0x01		/* User interface : can be used to render something */

	/* UI specifics 
	 * Example :
	 * 	LCD 1602 : SELCAP_RENDERER | SELCAPUI_DB
	 * 		- A simple monochrom textual display
	 * 	CURSE : SELCAP_RENDERER | SELCAPUI_COLOR
	 * 		- text based
	 * 		- color can be changed as well : N&B or color depending
	 * 		on terminal capabilities.
	 * 	OLED : SELCAP_RENDERER | SELCAPUI_HRGFX | SELCAPUI_COLOR
	 * 		- graphical display where color can be set (N&B)
	 * 	HDMI : SELCAP_RENDERER | SELCAPUI_HRGFX | SELCAPUI_COLOR
	 * 		- graphical display where color can be set (true colors)
	 */
#define SELCAPUI_HRGFX		0x10000	/* Can display graphics (textual otherwise) */
#define SELCAPUI_COLOR		0x20000	/* color can be set */
#define SELCAPUI_BUFFERED	0x40000	/* Buffered */
#define SELCAPUI_DB			0x80000	/* Double buffering*/

	/* All exported rendering stuffs
	 * All methods returns false in case of error or if not supported
	 */

/* *****
 * Graphical objects
 *
 * All graphical objects are derived from surfaces.
 * On limited ones where subsurface can't be created,
 * only the primary surface is available ... but it's a surface
 * as well.
 * ****/

struct SelGenericSurface;
struct SelCoordinate;

	/* Need to be implemented by derived modules */
struct RestrictArea;

	/* All callbacks shared among surfaces */
struct SGS_callbacks {
			/* Get Lua class name */
	const char * const (*LuaObjectName)();	/* Null if not exposed at Lua side */

		/* surface's */
	bool (*getSize)(struct SelGenericSurface *, uint32_t *width, uint32_t *height);
	struct SelGenericSurface *(*subSurface)(struct SelGenericSurface *parent, uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *primary);
	void *(*getPrimary)(struct SelGenericSurface *);
	void *(*getParent)(struct SelGenericSurface *);

		/* Text cursor / positioning */
	bool (*Home)(struct SelGenericSurface *);
	bool (*setCursor)(struct SelGenericSurface *, uint32_t x, uint32_t y);
	bool (*getCursor)(struct SelGenericSurface *, uint32_t *x, uint32_t *y);
	bool (*inSurface)(struct SelGenericSurface *, uint32_t x, uint32_t y);	/* Is (x,y) part of the surface */

		/* Graphics
		 * To prevent concurrent rush access, these actions are
		 * lock() protected.
		 */
	bool (*Clear)(struct SelGenericSurface *);
	bool (*WriteString)(struct SelGenericSurface *, const char *);

		/* Locking.
		 * Some devices don't support concurrent access. Locking prevents
		 * this kind of situation.
		 * Obviously, atomic sections HAVE TO BE AS SHORT AS POSSIBLE.
		 * By default, doing nothing.
		 *
		 * Some devices (like LCD's HD44780) need some time to proceed.
		 * In case of race condition, it's up to Lock() to ensure this
		 * timing is respected.
		 * heavy : identify when we need the longest timing.
		 */
	bool (*Lock)(struct SelGenericSurface *, bool heavy);
	bool (*Unlock)(struct SelGenericSurface *);

		/* buffering */
	bool (*AllocateBuffer)(struct SelGenericSurface *);
	bool (*Refresh)(struct SelGenericSurface *);			/* update the device as per (active) buffer's content */
	bool (*Dump)(struct SelGenericSurface *);	/* Dump/provide information about buffers */
	bool (*bSet)(struct SelGenericSurface *, const char, struct SelCoordinate *);	/* Put a char on surface's buffer but without moving the cursor */

		/* Restriction :
		 * To support multithreaded applications (e.g., Majordome), I replaced
		 * the global mask approach with a per-action mask to prevent concurrency
		 * issues.
		 * Note that, at present, this only affects buffered functions.
		 */
	bool (*inRA)(struct RestrictArea *, uint32_t x, uint32_t y);	/* Is (x,y) part of the surface */
	void (*getFootprint)(struct SelGenericSurface *, struct RestrictArea *);
	bool (*rbClear)(struct SelGenericSurface *, struct RestrictArea *);
	bool (*rbWriteString)(struct SelGenericSurface *, struct RestrictArea *, const char *);
};

struct SelGenericSurface {
	struct SelObject object;
	struct SGS_callbacks *cb;
};

struct SelGenericSurfaceLua {
	struct SelGenericSurface *storage;
};

#ifdef __cplusplus
}
#endif

#endif
