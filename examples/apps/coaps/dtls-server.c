/*
 * dtls-server.c
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#include <stdio.h>
#include <openthread/openthread.h>
#include "openthread/platform/alarm.h"

#include "dtls-server.h"

#ifdef OPENTHREAD_ENABLE_YACOAP
extern coap_resource_t resources[];
#endif

// Disable Logging without a CLI
#if OPENTHREAD_ENABLE_COAPS_CLI == 0
#define otPlatLog(...)
#endif

#ifdef OPENTHREAD_ENABLE_TINYDTLS
/* Handler that is called when a packet is received */
int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length)
{
	#ifndef NDEBUG
	char loggingBuffer[length];
	snprintf(loggingBuffer, sizeof loggingBuffer, "%s", data);
	otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d(MAIN): Received data (%d Byte)", otPlatAlarmGetNow(), length, loggingBuffer);
	#endif

#ifdef OPENTHREAD_ENABLE_YACOAP
	coap_packet_t requestPacket, responsePacket;
	uint8_t responseBuffer[DTLS_MAX_BUF];
	size_t responseBufferLength = sizeof(responseBuffer);

	if ((coap_parse(data, length, &requestPacket)) < COAP_ERR)
	{
		// Get data from resources
		coap_handle_request(resources, &requestPacket, &responsePacket);

		// Build response packet
		if ((coap_build(&responsePacket, responseBuffer, &responseBufferLength)) < COAP_ERR)
		{
			// Send response packet decrypted over DTLS
			dtls_write(context, session, responseBuffer, responseBufferLength);
		}
	}
#endif
	return 0;
}

/* Definition of executed handlers */
dtls_handler_t dtls_callback_server = {
  .write = handle_write,
  .read  = handle_read,
  .event = handle_event,
#ifdef DTLS_PSK
  .get_psk_info = get_psk_info,
#endif
};
#endif
