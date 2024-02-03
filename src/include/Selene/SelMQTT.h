/* SelLog.h
 *
 * Logging facility
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELMQTT_VERSION
#include "Selene/libSelene.h"

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELMQTT_VERSION 1

#include <MQTTClient.h>	/* Paho MQTT library needed */

#ifdef __cplusplus
extern "C"
{
#endif

struct SelMQTT {
	struct SelModule module;

		/* Call backs */
};

#ifdef __cplusplus
}
#endif

#endif
