/*
 * dtls-base.c
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#include <stdio.h>
#include "openthread/udp.h"
#include <openthread/openthread.h>
#include "openthread/platform/alarm.h"

#include "dtls-base.h"
#include "measurement.h"

#if OPENTHREAD_ENABLE_UDPSERVER
#include "dtls-server.h"
#endif

#if OPENTHREAD_ENABLE_UDPCLIENT
#include "dtls-client.h"
#endif

extern otInstance *mInstance;
extern otSockAddr sockaddr;
extern otUdpSocket mSocket;

#if OPENTHREAD_ENABLE_TINYDTLS
extern dtls_context_t *the_context;
#endif

#if OPENTHREAD_ENABLE_YACOAP || OPENTHREAD_ENABLE_UDPSERVER
extern coap_resource_t resources[];
#endif

// Disable Logging without a CLI
#if OPENTHREAD_ENABLE_COAPS_CLI == 0
//#define otPlatLog(...)
#endif

#if OPENTHREAD_ENABLE_TINYDTLS
/* Definition of executed handlers */
dtls_handler_t dtls_callback = {
  .write = handle_write,
  .read  = handle_read,
  .event = handle_event,
#ifdef DTLS_PSK
  .get_psk_info = get_psk_info,
#endif
};

/* Handler that is called when a packet should be sent */
int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len)
{
	#ifndef NDEBUG
	char buffer[len];
	snprintf(buffer, sizeof buffer, "%s", data);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(WRITE): Sending data (%d Byte)", otPlatAlarmGetNow(), len, buffer);
	#endif

	// Sending DTLS encrypted application data over UDP
	send_message(ctx, session, data, len);

	return len;
}

/* Handler that is called when an event occurs */
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code)
{

#ifndef NDEBUG
  if (code == DTLS_EVENT_CONNECTED) {
    //dtls_connected = 1;
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(EVENT): Connected!", otPlatAlarmGetNow());
  }
  else if (code == DTLS_EVENT_CONNECT){
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(EVENT): Connecting...", otPlatAlarmGetNow());
  }else{
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(EVENT): Event occurred! (level %d, code %d )", otPlatAlarmGetNow(), level, code);
  }
#endif

	(void) ctx;
	(void) session;
	(void) level;
	(void) code;
	return 0;
}

/* Handler that is called when a packet is received */
int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length)
{
	#ifndef NDEBUG
	char loggingBuffer[length];
	snprintf(loggingBuffer, sizeof loggingBuffer, "%s", data);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(READ): Received data (%d Byte)", otPlatAlarmGetNow(), length, loggingBuffer);
	#endif

#if OPENTHREAD_ENABLE_YACOAP
#if OPENTHREAD_ENABLE_UDPSERVER
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
#if OPENTHREAD_ENABLE_UDPCLIENT
	MEASUREMENT_DTLS_TOTAL_OFF;
	coap_packet_t packet;
	coap_parse(data, length, &packet);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(COAP): Answer was: %.*s", otPlatAlarmGetNow(), packet.payload.len, (char *)packet.payload.p);
	(void) context;
	(void) session;
#endif
#endif

	return 0;
}
#endif

#if OPENTHREAD_ENABLE_UDPSERVER || OPENTHREAD_ENABLE_UDPCLIENT
/* Handler that is called when a raw UDP packet is receiver*/
void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
	MEASUREMENT_DTLS_READ_ON;
	// Get message payload
    uint8_t payload[DTLS_MAX_BUF];
    uint16_t payloadLength = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    otMessageRead(aMessage, otMessageGetOffset(aMessage), payload, payloadLength);

    // Set current session data
    session_t session;
    memset(&session, 0, sizeof(session_t));
    session.size = sizeof(session.addr);
    session.addr = aMessageInfo->mPeerAddr;
    session.messageInfo = *aMessageInfo;

    // Forward session and payload data to TinyDTLS
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(onUDP): Receiving data", otPlatAlarmGetNow());
#if OPENTHREAD_ENABLE_TINYDTLS
    dtls_handle_message(the_context, &session, payload, payloadLength);
#endif
    MEASUREMENT_DTLS_READ_OFF;

    (void) aContext;
}

/* Sends a new OpenThread message to a given address */
void send_message(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len)
{
	otMessage *message;

	// Create message and write payload
	message = otUdpNewMessage(mInstance, true);
	otMessageSetLength(message, len);
	otMessageWrite(message, 0, data, len);

	// Send packet to peer
	otUdpSend(&mSocket, message, &session->messageInfo);
	(void) ctx;
}
#endif
