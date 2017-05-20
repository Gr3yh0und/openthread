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

extern dtls_context_t *the_context;
dtls_handler_t dtls_callback;

void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
void send_message(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code);
int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length);

#endif /* EXAMPLES_APPS_COAPS_DTLS_BASE_H_ */
