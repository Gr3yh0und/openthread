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

#include "measurement.h"

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

// DTLS client defines
#define DTLS_CLIENT_MESSAGE_CYCLE_TIME 1000
#define DTLS_CLIENT_BUFFER_SIZE        32

/* UDP server and client */
#if OPENTHREAD_ENABLE_UDPSERVER || OPENTHREAD_ENABLE_UDPCLIENT
extern void onUdpPacket(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
#endif

#if OPENTHREAD_ENABLE_UDPCLIENT
	static session_t session;
#ifdef OPENTHREAD_ENABLE_YACOAP
	static uint8 buffer[DTLS_CLIENT_BUFFER_SIZE];
	static size_t bufferLength = sizeof(buffer);
#endif
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

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}

#if OPENTHREAD_ENABLE_UDPCLIENT
// Callback gets executed by timer
void sendClientMessage(){
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, ".");
	MEASUREMENT_DTLS_TOTAL_ON;
	MEASUREMENT_DTLS_WRITE_ON;
	dtls_write(the_context, &session, buffer, bufferLength);
	MEASUREMENT_DTLS_WRITE_OFF;
}
#endif

int main(int argc, char *argv[])
{
	// Create OpenThread instance
    otInstance *sInstance;
    PlatformInit(argc, argv);
    sInstance = otInstanceInit();
    assert(sInstance);

    // Check if command line interface should be enabled
#if OPENTHREAD_ENABLE_COAPS_CLI
    otCliUartInit(sInstance);
#endif

    // Check if diagnostics interface should be enabled
#if OPENTHREAD_ENABLE_DIAG
    otDiagInit(sInstance);
#endif

    // Enable basic hard coded Thread configuration
	otLinkSetPanId(sInstance, 0x1234);
	otIp6SetEnabled(sInstance, true);
	otThreadSetEnabled(sInstance, true);
	otLinkSetPollPeriod(sInstance, 300);

	// Add specific IP address for testing
	otNetifAddress aAddress;
	otIp6AddressFromString("fdde:ad00:beef:0:5d12:76b8:948e:5b42", &aAddress.mAddress);
	aAddress.mPrefixLength = 64;
	aAddress.mPreferred = true;
	aAddress.mValid = true;
	otIp6AddUnicastAddress(sInstance, &aAddress);

	// Initialise UDP
#if OPENTHREAD_ENABLE_UDPSERVER || OPENTHREAD_ENABLE_UDPCLIENT
	// Create variables
	memset(&mSocket, 0, sizeof(mSocket));
	memset(&sockaddr, 0, sizeof(otSockAddr));
	mInstance = sInstance;
	sockaddr.mPort = OPENTHREAD_UDP_PORT_LOCAL;

	// Bind Port
	otUdpOpen(sInstance, &mSocket, (otUdpReceive) &onUdpPacket, &mSocket);
	otUdpBind(&mSocket, &sockaddr);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "Socket open...");
#endif

	// Initialise DTLS & CoAP
#if OPENTHREAD_ENABLE_TINYDTLS
	// Initialise DTLS basics and setting log level
	dtls_init();
	dtls_set_log_level(DTLS_LOG_LEVEL);

	// Create server context and set dtls handler
	the_context = dtls_new_context(&mSocket);
	dtls_set_handler(the_context, &dtls_callback);

	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "DTLS initialized...");

#if OPENTHREAD_ENABLE_YACOAP && OPENTHREAD_ENABLE_UDPSERVER
	// Initialise CoAP server resources
	resource_setup(resources);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "CoAP resources initialised...");
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "Server started...");
#endif

#endif

	// Configure UDP Client
#if OPENTHREAD_ENABLE_UDPCLIENT
	// define session
	memset(&session, 0, sizeof(session_t));
	session.size = sizeof(session.addr);

	// define source and destination
	otMessageInfo dest_messageInfo;
	otIp6AddressFromString("fdde:ad00:beef:0:eaa5:a161:2dd9:aa41", &session.addr);
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

	// Check which function the CoAP client should have
#ifdef WITH_CLIENT_PUT
	// PUT light
	static coap_resource_path_t resourcePath = {1, {"status"}};
	static coap_resource_t request = {COAP_RDY, COAP_METHOD_PUT, COAP_TYPE_CON, NULL, &resourcePath, COAP_SET_CONTENTTYPE(COAP_CONTENTTYPE_TXT_PLAIN)};
	coap_make_request(messageId, NULL, &request, &messageId, sizeof(messageId), &requestPacket);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "Client mode = PUT");
#else
	// GET time
	static coap_resource_path_t resourcePath = {1, {"status"}};
	static coap_resource_t request = {COAP_RDY, COAP_METHOD_GET, COAP_TYPE_CON, NULL, &resourcePath, COAP_SET_CONTENTTYPE(COAP_CONTENTTYPE_TXT_PLAIN)};
	coap_make_request(messageId, NULL, &request, NULL, 0, &requestPacket);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "Client mode = GET");
#endif // WITH_CLIENT_PUT

	coap_build(&requestPacket, buffer, &bufferLength);
#endif // OPENTHREAD_ENABLE_YACOAP

	// Set Timer Callback function
	otTimerSetCallback(sInstance, &sendClientMessage);
	otPlatLog(OT_LOG_LEVEL_DEBG, OT_LOG_REGION_PLATFORM, "Client started...");
#endif

	// Check if GPIO should be enabled or not
#if GPIO_OUTPUT_ENABLE
	measurement_init_gpio();
#endif

    while (1)
    {
        otTaskletsProcess(sInstance);
        PlatformProcessDrivers(sInstance);

#if OPENTHREAD_ENABLE_UDPCLIENT
        // Reset timer
		if(!otTimerIsRunning(sInstance))
        	otTimerStart(sInstance, DTLS_CLIENT_MESSAGE_CYCLE_TIME);
#endif
    }

    return 0;
}