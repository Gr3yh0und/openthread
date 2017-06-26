/*
 * dtls-base.h
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#ifndef EXAMPLES_APPS_COAPS_DTLS_BASE_H_
#define EXAMPLES_APPS_COAPS_DTLS_BASE_H_

#include "../third_party/yacoap/coap.h"
#include "../third_party/tinydtls/dtls.h"
#include "../third_party/tinydtls/dtls_debug.h"
#include "measurement.h"

#if WITH_SERVER
#include "dtls-server.h"
#endif

#if WITH_CLIENT
#include "dtls-client.h"
#endif

// Define default Port of local UDP server
#ifndef OPENTHREAD_UDP_PORT_LOCAL
#define OPENTHREAD_UDP_PORT_LOCAL 6666
#endif

// Define default Port of remote UDP server
#ifndef OPENTHREAD_UDP_PORT_REMOTE
#define OPENTHREAD_UDP_PORT_REMOTE 7777
#endif

// DTLS client defines
#define DTLS_CLIENT_MESSAGE_CYCLE_TIME 1000
#define DTLS_CLIENT_BUFFER_SIZE        32
#define DTLS_LOG_LEVEL                 DTLS_LOG_WARN

extern dtls_context_t *the_context;
dtls_handler_t dtls_callback;

void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
void send_message(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code);
int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length);

#endif /* EXAMPLES_APPS_COAPS_DTLS_BASE_H_ */
