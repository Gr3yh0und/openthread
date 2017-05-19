/*
 * dtls-base.h
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#ifndef EXAMPLES_APPS_COAPS_DTLS_BASE_H_
#define EXAMPLES_APPS_COAPS_DTLS_BASE_H_

#include "../third_party/tinydtls/dtls.h"
#include "../third_party/tinydtls/dtls_debug.h"

extern dtls_context_t *the_context;

int get_psk_info(struct dtls_context_t *ctx, const session_t *session,
						 dtls_credentials_type_t type,
						 const unsigned char *id, size_t id_len,
						 unsigned char *result, size_t result_length);
void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
void send_message(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len);
int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code);


#endif /* EXAMPLES_APPS_COAPS_DTLS_BASE_H_ */
