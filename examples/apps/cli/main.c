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

#include <assert.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/platform/platform.h>
#include <openthread/openthread.h>

#include <string.h>

#include "openthread/udp.h"
#include "../third_party/tinydtls/dtls.h"
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

otInstance *mInstance;
otUdpSocket mSocket;
otSockAddr sockaddr;
dtls_context_t *the_context = NULL;

int test(struct dtls_context_t *ctx, session_t *session, uint8 *data, size_t len){
	(void) ctx;
	(void) session;
	(void) data;
	(void) len;
	return 1;
}

static dtls_handler_t cb = {
  .write = test,
  .read  = test,
  .event = NULL,
  .get_psk_info = NULL,
};

/*int dtls_handle_read(struct dtls_context_t *ctx) {
  int *fd;
  session_t session;
  static uint8 buf[DTLS_MAX_BUF];
  int len;
  fd = dtls_get_app_data(ctx);
  assert(fd);
  session.size = sizeof(session.addr);
  len = recvfrom(*fd, buf, sizeof(buf), 0, &session.addr.sa, &session.size);

  return len < 0 ? len : dtls_handle_message(ctx, &session, buf, len);
}*/


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

    // Initialize OpenThread with the buffer
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
