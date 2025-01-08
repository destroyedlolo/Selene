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
#define SELLCD_VERSION 2

struct SelLCDScreen;

struct SelLCD {
	struct SelModule module;

		/* ***
		 * Callbacks 
		 * ***/
	
		/* Low level functions */
	void (*SendQuarter)(struct SelLCDScreen *, uint8_t);
	void (*SendCmd)(struct SelLCDScreen *, uint8_t);
	void (*SendData)(struct SelLCDScreen *, uint8_t);

		/* Screen level functions
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
};

#ifdef __cplusplus
}
#endif

#endif
