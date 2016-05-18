/* MQTT_tools.c
 *
 * Tools useful for MQTT processing
 * (this file is shared by several projects)
 *
 * 13/07/2015 LF : First version
 */

#include "MQTT_tools.h"

int mqtttokcmp(register const char *s, register const char *t){
	char last = 0;
	if(!s || !t)
		return -1;

	for(; ; s++, t++){
		if(!*s){ /* End of string */
			return(*s - *t);
		} else if(*s == '#') /* ignore remaining of the string */
			return (!*++s && (!last || last=='/')) ? 0 : -1;
		else if(*s == '+'){	/* ignore current level of the hierarchy */
			s++;
			if((last !='/' && last ) || (*s != '/' && *s )) /* has to be enclosed by '/' */
				return -1;
			while(*t != '/'){	/* looking for the closing '/' */
				if(!*t)
					return 0;
				t++;
			}
			if(*t != *s)	/* ensure the s isn't finished */
				return -1;
		} else if(*s != *t)
			break;
		last = *s;
	}
	return(*s - *t);
}

int mqttpublish(MQTTClient client, const char *topic, int length, void *payload, int retained ){ /* Custom wrapper to publish */
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.retained = retained;
	pubmsg.payloadlen = length;
	pubmsg.payload = payload;

	return MQTTClient_publishMessage(client, topic, &pubmsg, NULL);
}

