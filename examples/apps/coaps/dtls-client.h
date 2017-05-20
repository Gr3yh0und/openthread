/*
 * dtls-client.h
 *
 *  Created on: 17 May 2017
 *      Author: gr3yh0und
 */

#ifndef EXAMPLES_APPS_COAPS_DTLS_CLIENT_H_
#define EXAMPLES_APPS_COAPS_DTLS_CLIENT_H_

#include "dtls-base.h"

int get_psk_info(struct dtls_context_t *ctx,
                        const session_t *session,
                        dtls_credentials_type_t type,
                        const unsigned char *id, size_t id_len,
                        unsigned char *result, size_t result_length);

#endif /* EXAMPLES_APPS_COAPS_DTLS_CLIENT_H_ */
