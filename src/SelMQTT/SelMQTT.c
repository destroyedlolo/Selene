/***
MQTT messaging including connection part.

Have a look on **SeleMQTT** when the connection has to be managed externally.

@classmod SelMQTT

 * History :
 * 30/05/2015 LF : First version
 * 17/06/2015 LF : Add trigger function to topic
 * 11/11/2015 LF : Add TaskOnce enum
 * 21/01/2015 LF : Rename as SelMQTT
 * 11/04/2021 LF : add retained and dupplicate parameters to callback receiving function
 * 03/02/2024 LF : Switch to Selene v7
 
 */

#include "Selene/SelMQTT.h"
#include "Selene/SeleneCore.h"
#include "Selene/SelLog.h"

static struct SelMQTT selMQTT;

static struct SeleneCore *seleneCore;
static struct SelLog *selLog;

/* ***
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
bool InitModule( void ){

		/* Initialise module's glue */
	if(!initModule((struct SelModule *)&selMQTT, "SelMQTT", SELMQTT_VERSION, LIBSELENE_VERSION))
		return false;

	registerModule((struct SelModule *)&selMQTT);

	return true;
}
