/* SelLCD.h
 *
 * Display messages on an LCD textual screen (like 1602 one)
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELLCD_VERSION

#ifdef __cplusplus
extern "C"
{
#endif

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

#include <unistd.h>

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELLCD_VERSION 6

struct SelLCDScreen;
struct SelLCDCoordinate;

struct SelLCD {
	struct SelModule module;

		/* ***
		 * Callbacks
		 * ***
		 * Some functions offer multiple variants depending on their
		 * abstraction level and the entities they target (e.g., buffer or
		 * physical screen).
		 *
		 * Application level function
		 * ---------------------
		 *
		 *  High-level API for direct screen control bypassing the Toile
		 * framework : These functions allow low-level manual management of
		 * text positioning and drawing. They perform operations on both the
		 * physical screen and the internal buffer.
		 *
		 * 	- Clear()
		 * 	- WriteString()
		 *
		 * Buffer Level Functions
		 * ---------------------
		 *  Operations restricted to the "working_buffer" : These functions
		 * modify the off-screen buffer only and do not trigger immediate
		 * physical screen updates.
		 *
		 *	- bClear()
		 *	- bWriteString()
		 *	- bSet()
		 *
		 * Physical Level Functions
		 * ---------------------
		 *  Direct hardware abstraction layer : Primarily used internally for
		 *  synchronization between the buffer and the hardware.
		 * These functions are not exposed to the Lua API.
		 *
		 * 	- pClear()
		 * 	- pWriteString()
		 *
		 * ***/
	
		/* Low level functions */
	void (*SendQuarter)(struct SelLCDScreen *, uint8_t);
	void (*SendCmd)(struct SelLCDScreen *, uint8_t);
	void (*SendData)(struct SelLCDScreen *, uint8_t);

		/* Direct screen functions
		 *
		 * These functions can be directly used to control the screen
		 * without Toile framework on top of it.
		 */
	bool (*Init)(struct SelLCDScreen *, uint16_t bus_number, uint8_t address, bool twolines, bool y11);
	void (*Shutdown)(struct SelLCDScreen *);

	void (*SetSize)(struct SelLCDScreen *, uint32_t, uint32_t);
	bool (*GetSize)(struct SelLCDScreen *, uint32_t *, uint32_t *);

	void (*Backlight)(struct SelLCDScreen *, bool);
	void (*DisplayCtl)(struct SelLCDScreen *, bool screen, bool cursor, bool blink);
	void (*EntryCtl)(struct SelLCDScreen *, bool inc, bool shift);
	void (*Clear)(struct SelLCDScreen *);
	bool (*Home)(struct SelLCDScreen *);
	void (*SetDDRAM)(struct SelLCDScreen *, uint8_t);
	void (*SetCGRAM)(struct SelLCDScreen *, uint8_t);
	bool (*SetCursor)(struct SelLCDScreen *, uint16_t, uint16_t);
	void (*WriteString)(struct SelLCDScreen *, const char *);

		/* Buffering's
		 *
		 * These functions are acting on the working buffer, without
		 * modifying the screen (until Refresh()).
		 */
	void (*Refresh)(struct SelLCDScreen *);

	void (*bSet)(struct SelLCDScreen *, const char, struct SelLCDCoordinate *);
	void (*bClear)(struct SelLCDScreen *);
	void (*bWriteString)(struct SelLCDScreen *, const char *);
	void (*DumpBuffers)(struct SelLCDScreen *);

		/* Physical's
		 *
		 * To be used internaly to refresh the screen
		 */
	void (*pClear)(struct SelLCDScreen *);
	void (*pWriteString)(struct SelLCDScreen *, const char *);
};

#ifdef __cplusplus
}
#endif

#endif
