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
	 * 	LCD 1602 : SELCAP_RENDERER
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
#define SELCAPUI_HRGFX	0x10000	/* Can display graphics (textual otherwise) */
#define SELCAPUI_COLOR	0x20000	/* color can be set */

	/* All exported rendering stuffs
	 * All methods returns false in case of error or if not supported
	 */

/* *****
 * Graphical objects
 *
 * All graphical objects are derived from surfaces.
 * On limited ones where subsurface can't be created like 1602,
 * only the primary surface is available ... but it's a surface
 * as well.
 * ****/

struct SelGenericSurface;

	/* All callbacks shared among surfaces */
struct SGS_callbacks {
			/* Get Lua class name */
	const char * const (*LuaObjectName)();	/* Null if not exposed at Lua side */

		/* surface's */
	bool (*getSize)(struct SelGenericSurface *, uint16_t *width, uint16_t *height);
	struct SelGenericSurface *(*subSurface)(struct SelGenericSurface *parent, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *primary);

		/* Text cursor / positioning */
	bool (*Home)(struct SelGenericSurface *);
	bool (*setCursor)(struct SelGenericSurface *, uint16_t x, uint16_t y);
	bool (*getCursor)(struct SelGenericSurface *, uint16_t *x, uint16_t *y);
	bool (*inSurface)(struct SelGenericSurface *, uint16_t x, uint16_t y);	/* Is (x,y) part of the surface */

		/* Graphics */
	bool (*Clear)(struct SelGenericSurface *);
	bool (*WriteString)(struct SelGenericSurface *, const char *);
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
