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

#include "openthread/udp.h"
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/platform/platform.h>
#include <openthread/openthread.h>

#include <string.h>
#include <stdio.h>

#include "dtls-server.h"

// Define default Port of UDP server
#ifndef OPENTHREAD_UDP_PORT
#define OPENTHREAD_UDP_PORT 6666
#endif

/* UDP server variables*/
#if OPENTHREAD_ENABLE_UDPSERVER
extern void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
#endif

/* TinyDTLS variables */
#if OPENTHREAD_ENABLE_TINYDTLS
#define DTLS_LOG_LEVEL DTLS_LOG_WARN
otInstance *mInstance;
otSockAddr sockaddr;
otUdpSocket mSocket;
dtls_context_t *the_context = NULL;
#endif

/* YaCoAP variables */
#if OPENTHREAD_ENABLE_YACOAP
extern void resource_setup(const coap_resource_t *resources);
extern coap_resource_t resources[];
#endif

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

    // Initialise OpenThread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInit();
#endif
    assert(sInstance);

#if OPENTHREAD_ENABLE_COAPS_CLI
    otCliUartInit(sInstance);
#endif

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

#if OPENTHREAD_ENABLE_UDPSERVER
	// Create variables
	memset(&mSocket, 0, sizeof(mSocket));
	memset(&sockaddr, 0, sizeof(otSockAddr));
	mInstance = sInstance;
	sockaddr.mPort = OPENTHREAD_UDP_PORT;

	// Bind Port
	otUdpOpen(sInstance, &mSocket, (otUdpReceive) &onUdpPacket, &mSocket);
	otUdpBind(&mSocket, &sockaddr);
#endif

#if OPENTHREAD_ENABLE_TINYDTLS
	// Initialise DTLS basics and setting log level
	dtls_init();
	dtls_set_log_level(DTLS_LOG_LEVEL);
#if OPENTHREAD_ENABLE_UDPSERVER
	// Create server context
	the_context = dtls_new_context(&mSocket);
	dtls_set_handler(the_context, &dtls_callback);
#if OPENTHREAD_ENABLE_YACOAP
	// Initialise CoAP server resources
	resource_setup(resources);
#endif
#else
	// Create client context
#endif
#endif

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
