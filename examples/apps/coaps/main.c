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

#if OPENTHREAD_ENABLE_GPIO
#include "gpio.h"
#endif

#include <string.h>
#include <stdio.h>

#if OPENTHREAD_ENABLE_UDPSERVER || OPENTHREAD_ENABLE_UDPCLIENT
#include "dtls-base.h"
#include "dtls-client.h"
#endif

// Define default Port of local UDP server
#ifndef OPENTHREAD_UDP_PORT_LOCAL
#define OPENTHREAD_UDP_PORT_LOCAL 6666
#endif

// Define default Port of remote UDP server
#ifndef OPENTHREAD_UDP_PORT_REMOTE
#define OPENTHREAD_UDP_PORT_REMOTE 7777
#endif

/* UDP server and client */
#if OPENTHREAD_ENABLE_UDPSERVER || OPENTHREAD_ENABLE_UDPCLIENT
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

#if OPENTHREAD_ENABLE_UDPSERVER || OPENTHREAD_ENABLE_UDPCLIENT
	// Create variables
	memset(&mSocket, 0, sizeof(mSocket));
	memset(&sockaddr, 0, sizeof(otSockAddr));
	mInstance = sInstance;
	sockaddr.mPort = OPENTHREAD_UDP_PORT_LOCAL;

	// Bind Port
	otUdpOpen(sInstance, &mSocket, (otUdpReceive) &onUdpPacket, &mSocket);
	otUdpBind(&mSocket, &sockaddr);
#endif

#if OPENTHREAD_ENABLE_TINYDTLS
	// Initialise DTLS basics and setting log level
	dtls_init();
	dtls_set_log_level(DTLS_LOG_LEVEL);

	// Create server context and set dtls handler
	the_context = dtls_new_context(&mSocket);
	dtls_set_handler(the_context, &dtls_callback);

#if OPENTHREAD_ENABLE_YACOAP
	// Initialise CoAP server resources
	resource_setup(resources);
#endif

#endif


#if OPENTHREAD_ENABLE_UDPCLIENT
	int counter_start = 500000;

	// define session
	session_t session;
	memset(&session, 0, sizeof(session_t));
	session.size = sizeof(session.addr);

	// define source and destination
	otMessageInfo dest_messageInfo;
	otIp6AddressFromString("fdde:ad00:beef:0:5122:e9d5:7ab8:c42f", &session.addr);
	dest_messageInfo.mHopLimit = 8;
	dest_messageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
	dest_messageInfo.mSockPort = mSocket.mSockName.mPort;
	dest_messageInfo.mSockAddr = mSocket.mSockName.mAddress;
	dest_messageInfo.mPeerPort = OPENTHREAD_UDP_PORT_REMOTE;
	dest_messageInfo.mPeerAddr = session.addr;
	session.messageInfo = dest_messageInfo;

#ifdef OPENTHREAD_ENABLE_YACOAP
	static coap_packet_t requestPacket;
	static uint8 messageId = 42;
	static uint8 buffer[32];
	static size_t bufferLength = sizeof(buffer);

#ifdef WITH_CLIENT_PUT
	// PUT light
	static coap_resource_path_t resourcePath = {1, {"light"}};
	static coap_resource_t request = {COAP_RDY, COAP_METHOD_PUT, COAP_TYPE_CON, NULL, &resourcePath, COAP_SET_CONTENTTYPE(COAP_CONTENTTYPE_TXT_PLAIN)};
	coap_make_request(messageId, NULL, &request, &messageId, sizeof(messageId), &requestPacket);
#else
	// GET time
	static coap_resource_path_t resourcePath = {1, {"time"}};
	static coap_resource_t request = {COAP_RDY, COAP_METHOD_GET, COAP_TYPE_CON, NULL, &resourcePath, COAP_SET_CONTENTTYPE(COAP_CONTENTTYPE_TXT_PLAIN)};
	coap_make_request(messageId, NULL, &request, NULL, 0, &requestPacket);
#endif

	coap_build(&requestPacket, buffer, &bufferLength);
#endif

#if OPENTHREAD_ENABLE_GPIO
	cc2538LedsInit();
#endif

	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "Client Started...");
#endif

    while (1)
    {
        otTaskletsProcess(sInstance);
        PlatformProcessDrivers(sInstance);

#if OPENTHREAD_ENABLE_UDPCLIENT
        counter_start--;
        if(counter_start == 0){
          counter_start = 300000;
          otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "New round...");

          // switch on LED
#ifdef OPENTHREAD_ENABLE_GPIO
          LED2_ON;
#endif

          // send data
          dtls_write(the_context, &session, buffer, bufferLength);

          // switch off LED
#ifdef OPENTHREAD_ENABLE_GPIO
          LED2_OFF;
#endif
        }
#endif
    }

    // otInstanceFinalize(sInstance);
#ifdef OPENTHREAD_MULTIPLE_INSTANCE
    // free(otInstanceBuffer);
#endif

    return 0;
}