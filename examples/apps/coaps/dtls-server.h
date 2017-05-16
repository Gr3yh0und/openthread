/*
 * dtls-server.h
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#ifndef EXAMPLES_APPS_COAPS_DTLS_SERVER_H_
#define EXAMPLES_APPS_COAPS_DTLS_SERVER_H_

#include "../third_party/microcoap/coap.h"
#include "dtls-base.h"

void send_message(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length);
int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code);
void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);

dtls_handler_t dtls_callback;

#endif /* EXAMPLES_APPS_COAPS_DTLS_SERVER_H_ */
