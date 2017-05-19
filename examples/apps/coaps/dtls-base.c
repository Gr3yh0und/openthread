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

#ifdef OPENTHREAD_ENABLE_TINYDTLS
extern dtls_context_t *the_context;
#endif

// Disable Logging without a CLI
#if OPENTHREAD_ENABLE_COAPS_CLI == 0
#define otPlatLog(...)
#endif

#if defined(OPENTHREAD_ENABLE_TINYDTLS) && defined(DTLS_PSK)
/* This function is the "key store" for tinyDTLS. It is called to
 * retrieve a key for the given identity within this particular
 * session. */
int get_psk_info(struct dtls_context_t *ctx, const session_t *session,
						 dtls_credentials_type_t type,
						 const unsigned char *id, size_t id_len,
						 unsigned char *result, size_t result_length) {
	struct keymap_t {
		unsigned char *id;
		size_t id_length;
		unsigned char *key;
		size_t key_length;
	} psk[3] = {
			{ (unsigned char *)"Client_identity", 15, (unsigned char *)"secretPSK", 9 },
			{ (unsigned char *)"default identity", 16, (unsigned char *)"\x11\x22\x33", 3 },
			{ (unsigned char *)"\0", 2, (unsigned char *)"", 1 }
	};

	if (type != DTLS_PSK_KEY) {
		return 0;
	}

	if (id) {
		uint8_t i;
		for (i = 0; i < sizeof(psk)/sizeof(struct keymap_t); i++) {
			if (id_len == psk[i].id_length && memcmp(id, psk[i].id, id_len) == 0) {
				if (result_length < psk[i].key_length) {
					return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
				}

				memcpy(result, psk[i].key, psk[i].key_length);
				return psk[i].key_length;
			}
		}
	}
	(void) session;
	(void) ctx;
	return dtls_alert_fatal_create(DTLS_ALERT_DECRYPT_ERROR);
}
#endif /* DTLS_PSK */

/* Handler that is called when a raw UDP packet is receiver*/
void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
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
    otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d(MAIN): Receiving data", otPlatAlarmGetNow());
#ifdef OPENTHREAD_ENABLE_TINYDTLS
#ifdef OPENTHREAD_ENABLE_UDPSERVER
    dtls_handle_message(the_context, &session, payload, payloadLength);
#endif
#ifdef OPENTHREAD_ENABLE_UDPCLIENT
    dtls_handle_message(the_context, &session, payload, payloadLength);
#endif
#endif

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

/* Handler that is called when a packet should be sent */
int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len)
{
	#ifndef NDEBUG
	char buffer[len];
	snprintf(buffer, sizeof buffer, "%s", data);
	otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d(MAIN): Sending data (%d Byte)", otPlatAlarmGetNow(), len, buffer);
	#endif

	// Sending DTLS encrypted application data over UDP
	send_message(ctx, session, data, len);

	return len;
}

/* Handler that is called when an event occurs */
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code)
{
	#ifndef NDEBUG
	otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d(MAIN): Event occurred! (level %d, code %d )", otPlatAlarmGetNow(), level, code);
	#endif

	// Handle event here

	(void) ctx;
	(void) session;
	(void) level;
	(void) code;
	return 0;
}
