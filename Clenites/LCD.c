/* Demonstration of Séléné LCD module.
 * Display all native characters
 *
 * 08/09/2024 - First version
 */

/* Basic modules needed by almost all applications */
#include <Selene/libSelene.h>	/* Modules : the only part hardly linked */
#include <Selene/SeleneCore.h>	/* Selene's core functionalities */
#include <Selene/SelLog.h>		/* Logging : not really mandatory but very useful in most of the cases */

#include <Selene/SelPlug-in/SelLCD.h>

	/* Here start 'standard' C code */
#include <dlfcn.h>		/* dlerror(), ... */
#include <stdlib.h>		/* exit(), ... */
#include <string.h>

int main( int ac, char ** av){
	uint16_t verfound;

		/* Load core functionalities */
	struct SeleneCore *SeleneCore = (struct SeleneCore *)loadModule("SeleneCore", SELENECORE_VERSION, &verfound);
#ifdef DEBUG
	printf("*D* SeleneCore %s : version %u\n", SeleneCore ? "found":"not found", verfound);
#endif
	if(!SeleneCore){	/* Needs to do checks manually as SeleneCore is ... not loaded */
		printf("*F* : can't load SeleneCore ");
		if(verfound)
			printf("(%u instead of expected %u)\n", verfound, SELENECORE_VERSION);
		else {
			char *err = dlerror();
			if(!err)
				puts(" : missing InitModule() or newer SelModule expected");
			else
				printf("(%s)\n", dlerror());
		}

		exit(EXIT_FAILURE);
	}

	struct SelLog *SelLog = (struct SelLog *)SeleneCore->loadModule("SelLog", SELLOG_VERSION, &verfound, 'F');

		/* We still need to do it manually as SeleneCore->loadModule() still can't logging */
	if(!SelLog){
		printf("*F* : can't load SelLog ");
		if(verfound)
			printf("(%u instead of expected %u)\n", verfound, SELLOG_VERSION);
		else {
			char *err = dlerror();
			if(!err)
				puts(" : missing InitModule() or outdated dependency found");
			else
				printf("(%s)\n", dlerror());
		}

		exit(EXIT_FAILURE);
	}

		/* ***
		 * Here your application code
		 * ***/

		/* Load SelLCD module ... using SeleneCore's facilities as logging is enabled */
	struct SelLCD *SelLCD = (struct SelLCD *)SeleneCore->loadModule("SelLCD", SELLCD_VERSION, &verfound, 'F');
	if(!SelLCD)
		exit(EXIT_FAILURE);

		/* LCD own code starting here */
	uint16_t nbus = 2;		/* BananaPI bus by default */
	uint8_t addr = 0x27;	/* screen address */
	bool verbose = false;

	int c;
	while((c = getopt(ac, av, "hvb:a:")) != EOF) switch(c){
	case 'b':
		nbus = atoi(optarg);
		break;
	case 'a':
		addr = atoi(optarg);
		break;
	case 'v':
		verbose = true;
		break;
	case 'h':
	default :
		printf("Known options :\n"
			"\t-b : i2c bus number (default 2)\n"
			"\t-a ! device's address (default 0x27)\n"
			"\t-v : be verbose\n"
		);
		exit( c == 'h' ? EXIT_SUCCESS :  EXIT_FAILURE);
	}

	if(verbose)
		printf("Targeting slave 0x%02x on /dev/i2c-%d\n", addr, nbus);

	struct LCDscreen lcd;

	if(!SelLCD->Init(&lcd, nbus, addr, true, false))	/* 16x02 screen */
		exit(EXIT_FAILURE);

	SelLCD->Clear(&lcd);
	SelLCD->WriteString(&lcd, "Hello");
	SelLCD->Backlight(&lcd, true);	/* Backlight on */
	SelLCD->DisplayCtl(&lcd, true, true, true);		/* On, cursor blinking */
	getchar();

	SelLCD->DisplayCtl(&lcd, true, false, true);	/* Only the block blinking */
	for(uint16_t i = 0x00; i < 0x100; i += 0x10){
		char t[6];
		sprintf(t, "0x%02x", i);

			/* Write the characters' group */
		SelLCD->Clear(&lcd);
		SelLCD->WriteString(&lcd, t);

		SelLCD->SetCursor(&lcd, 0,1);	/* 2nd lines */
		t[1] = 0;

		for(uint8_t j = 0x00; j < 0x10; j++){
			*t = i+j;
			SelLCD->WriteString(&lcd, t);
		}

		SelLCD->SetCursor(&lcd, 15,0);	/* Display the cursor on the top right */
		getchar();
	}

		/* The end */
	SelLCD->Backlight(&lcd, false);
	SelLCD->Shutdown(&lcd);
}

