/*
 * dtls-server.h
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#ifndef EXAMPLES_APPS_COAPS_DTLS_SERVER_H_
#define EXAMPLES_APPS_COAPS_DTLS_SERVER_H_

#include "dtls-base.h"

int get_psk_info(struct dtls_context_t *ctx, const session_t *session,
						 dtls_credentials_type_t type,
						 const unsigned char *id, size_t id_len,
						 unsigned char *result, size_t result_length);

#endif /* EXAMPLES_APPS_COAPS_DTLS_SERVER_H_ */
