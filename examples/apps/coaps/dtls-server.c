/*
 * dtls-server.c
 *
 *  Created on: 16 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#include "dtls-server.h"

// Disable Logging without a CLI
#if OPENTHREAD_ENABLE_COAPS_CLI == 0
//#define otPlatLog(...)
#endif

#if OPENTHREAD_ENABLE_TINYDTLS && defined(DTLS_PSK) && OPENTHREAD_ENABLE_UDPSERVER
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
#endif
