/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include "openthread/openthread.h"
#include "openthread/cli.h"
#include "openthread/diag.h"
#include "openthread/platform/platform.h"

#include <openthread-core-config.h>
#include <assert.h>

#include "../third_party/tinydtls/dtls.h"
#include "openthread/udp.h"
#include "openthread/platform/alarm.h"
#include <string.h>

#ifdef OPENTHREAD_MULTIPLE_INSTANCE
void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}
#endif

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}

#ifdef DTLS_PSK
/* This function is the "key store" for tinyDTLS. It is called to
 * retrieve a key for the given identity within this particular
 * session. */
static int
get_psk_info(struct dtls_context_t *ctx, const session_t *session,
	     dtls_credentials_type_t type,
	     const unsigned char *id, size_t id_len,
	     unsigned char *result, size_t result_length) {

  struct keymap_t {
    unsigned char *id;
    size_t id_length;
    unsigned char *key;
    size_t key_length;
  } psk[3] = {
    { (unsigned char *)"Client_identity", 15,
      (unsigned char *)"secretPSK", 9 },
    { (unsigned char *)"default identity", 16,
      (unsigned char *)"\x11\x22\x33", 3 },
    { (unsigned char *)"\0", 2,
      (unsigned char *)"", 1 }
  };

  if (type != DTLS_PSK_KEY) {
    return 0;
  }

  if (id) {
    uint8_t i;
    for (i = 0; i < sizeof(psk)/sizeof(struct keymap_t); i++) {
      if (id_len == psk[i].id_length && memcmp(id, psk[i].id, id_len) == 0) {
	if (result_length < psk[i].key_length) {
	  //dtls_warn("buffer too small for PSK");
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

otInstance *mInstance;
otUdpSocket mSocket;
otSockAddr sockaddr;
dtls_context_t *the_context = NULL;

void send_message(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len){
	otMessage *message;

	// Create message and write payload
	message = otUdpNewMessage(mInstance, true);
	otMessageSetLength(message, len);
	otMessageWrite(message, 0, data, len);

	// Send packet to peer
	otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d: Sending Packet!", otPlatAlarmGetNow());
	otUdpSend(&mSocket, message, &session->messageInfo);
	(void) ctx;
}

int handle_read(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len){
	send_message(ctx, session, data, len);
	return 0;
}

int handle_write(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len){
	send_message(ctx, session, data, len);
	(void) ctx;
	return len;
}

int handle_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level, unsigned short code){
	otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d: event with level %d = %s ", otPlatAlarmGetNow(), level, code);
	(void) ctx;
	(void) session;
	return 0;
}

static dtls_handler_t dtls_callback = {
  .write = handle_write,
  .read  = handle_read,
  .event = handle_event,
  .get_psk_info = get_psk_info,
};

void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
	// Get message payload
    uint8_t buf[256];
    uint16_t payloadLength = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, payloadLength);

    // Set current session data
    session_t session;
    memset(&session, 0, sizeof(session_t));
    session.size = sizeof(session.addr);
    session.addr = aMessageInfo->mPeerAddr;
    session.messageInfo = *aMessageInfo;

    // Forward session and payload data to TinyDTLS
    otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d: Receiving Packet!", otPlatAlarmGetNow());
    dtls_handle_message(the_context, &session, buf, payloadLength);

    (void) aContext;
}

int main(int argc, char *argv[])
{
    otInstance *sInstance;

#ifdef OPENTHREAD_MULTIPLE_INSTANCE
    size_t otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer = NULL;
#endif

    PlatformInit(argc, argv);

#ifdef OPENTHREAD_MULTIPLE_INSTANCE
    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize Openthread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInit();
#endif
    assert(sInstance);

    otCliUartInit(sInstance);

#if OPENTHREAD_ENABLE_DIAG
    otDiagInit(sInstance);
#endif

    // Enable basic Thread config, ToDo: REMOVE THIS
	otLinkSetPanId(sInstance, 0x1234);
	otIp6SetEnabled(sInstance, true);
	otThreadSetEnabled(sInstance, true);

	// Add specific IP address, ToDo: REMOVE THIS
	otNetifAddress aAddress;
	otIp6AddressFromString("fdde:ad00:beef:0:5d12:76b8:948e:5b42", &aAddress.mAddress);
	aAddress.mPrefixLength = 64;
	aAddress.mPreferred = true;
	aAddress.mValid = true;
	otIp6AddUnicastAddress(sInstance, &aAddress);

	// Create variables
	memset(&mSocket, 0, sizeof(mSocket));
	memset(&sockaddr, 0, sizeof(otSockAddr));
	mInstance = sInstance;
	sockaddr.mPort = 6666; // ToDo: hardcoded

	// Bind Port
	otUdpOpen(sInstance, &mSocket, (otUdpReceive) &onUdpPacket, &mSocket);
	otUdpBind(&mSocket, &sockaddr);

	// Init DTLS
	dtls_init();
	the_context = dtls_new_context(&mSocket);
	dtls_set_handler(the_context, &dtls_callback);

    while (1)
    {
        otTaskletsProcess(sInstance);
        PlatformProcessDrivers(sInstance);
    }

    // otInstanceFinalize(sInstance);
#ifdef OPENTHREAD_MULTIPLE_INSTANCE
    // free(otInstanceBuffer);
#endif

    return 0;
}
