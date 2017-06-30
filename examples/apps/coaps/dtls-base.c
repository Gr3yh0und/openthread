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

extern otInstance *mInstance;
extern otSockAddr sockaddr;
extern otUdpSocket mSocket;

#if WITH_TINYDTLS
extern dtls_context_t *the_context;
#endif

#if WITH_YACOAP || WITH_SERVER
extern coap_resource_t resources[];
#endif

#if WITH_TINYDTLS
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
#if DEBUG
	char buffer[len];
	snprintf(buffer, sizeof buffer, "%s", data);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(WRITE): Sending data (%d Byte)", otPlatAlarmGetNow(), len, buffer);
#endif

	// Sending DTLS encrypted application data over UDP
    send_packet(session, data, len);
    (void) ctx;
    return len;
}

/* Handler that is called when an event occurs */
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code)
{

#if DEBUG
  if (code == DTLS_EVENT_CONNECTED) {
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(EVENT): Connected!", otPlatAlarmGetNow());
  }else if (code == DTLS_EVENT_CONNECT){
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
#if DEBUG
	char loggingBuffer[length];
	snprintf(loggingBuffer, sizeof loggingBuffer, "%s", data);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(READ): Received data (%d Byte)", otPlatAlarmGetNow(), length, loggingBuffer);
#endif

#if WITH_YACOAP
#if WITH_SERVER
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
			MEASUREMENT_DTLS_WRITE_ON;
			dtls_write(context, session, responseBuffer, responseBufferLength);
			MEASUREMENT_DTLS_WRITE_OFF;
		}
	}
#endif
#if WITH_CLIENT
	MEASUREMENT_DTLS_TOTAL_OFF;
	coap_packet_t packet;
	coap_parse(data, length, &packet);
#if DEBUG
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(COAP): Answer was: %.*s", otPlatAlarmGetNow(), packet.payload.len, (char *)packet.payload.p);
#endif
	(void) context;
	(void) session;
#endif
#endif

	return 0;
}
#endif

#if WITH_SERVER || WITH_CLIENT
/* Handler that is called when a raw UDP packet is receiver*/
void read_packet(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
	MEASUREMENT_DTLS_READ_ON;
#if WITH_SERVER
	MEASUREMENT_DTLS_TOTAL_ON;
#endif
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

#if DEBUG
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(onUDP): Receiving data", otPlatAlarmGetNow());
#endif

    MEASUREMENT_DTLS_READ_OFF;

    // Forward session and payload data
#if WITH_TINYDTLS
    // Secure
    dtls_handle_message(the_context, &session, payload, payloadLength);
#else
    // Unsecure
    handle_message(&session, payload, payloadLength);
#endif
#if WITH_SERVER
	MEASUREMENT_DTLS_TOTAL_OFF;
#endif

    (void) aContext;
}

/* Sends a new OpenThread message to a given address */
void send_packet(session_t *session, uint8 *data, size_t len)
{
#ifdef WITH_SERVER
	MEASUREMENT_DTLS_WRITE_ON;
#endif
	otMessage *message;

	// Create message and write payload
	message = otUdpNewMessage(mInstance, true);
	otMessageSetLength(message, len);
	otMessageWrite(message, 0, data, len);

	// Send packet to peer
	otUdpSend(&mSocket, message, &session->messageInfo);
#ifdef WITH_SERVER
	MEASUREMENT_DTLS_WRITE_OFF;
#endif
}

void handle_message(session_t *session, uint8 *message, int messageLength){
#if DEBUG
    otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "%d(handle_message): Received request...", otPlatAlarmGetNow());
#endif
#ifdef WITH_YACOAP
	coap_packet_t requestPacket, responsePacket;
	uint8_t responseBuffer[DTLS_MAX_BUF];
	size_t responseBufferLength = sizeof(responseBuffer);

	// Parse received packet for CoAP request
	if ((coap_parse(message, messageLength, &requestPacket)) < COAP_ERR)
	{
		// Get data from resources
		coap_handle_request(resources, &requestPacket, &responsePacket);

		// Build response packet
		if ((coap_build(&responsePacket, responseBuffer, &responseBufferLength)) < COAP_ERR)
		{
			// Send response packet
			send_packet(session, responseBuffer, responseBufferLength);
		}
	}
#endif
}
#endif
