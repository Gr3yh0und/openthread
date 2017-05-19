/*
 * dtls-server.h
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#ifndef EXAMPLES_APPS_COAPS_DTLS_SERVER_H_
#define EXAMPLES_APPS_COAPS_DTLS_SERVER_H_

#include "../third_party/yacoap/coap.h"
#include "dtls-base.h"

int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length);

dtls_handler_t dtls_callback_server;

#endif /* EXAMPLES_APPS_COAPS_DTLS_SERVER_H_ */
