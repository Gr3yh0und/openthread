/*
 * dtls-client.c
 *
 *  Created on: 17 May 2017
 *      Author: gr3yh0und
 */

#ifdef OPENTHREAD_ENABLE_TINYDTLS
/* Definition of executed handlers */
dtls_handler_t dtls_callback_client = {
  .write = handle_write,
  .read  = handle_read,
  .event = handle_event,
#ifdef DTLS_PSK
  .get_psk_info = get_psk_info,
#endif
};

  /* Handler that is called when a packet is received */
int handle_read(struct dtls_context_t *context, session_t *session, uint8 *data, size_t length)
{
	#ifndef NDEBUG
	char loggingBuffer[length];
	snprintf(loggingBuffer, sizeof loggingBuffer, "%s", data);
	otPlatLog(kLogLevelDebg, kLogRegionPlatform, "%d(MAIN): Received data (%d Byte)", otPlatAlarmGetNow(), length, loggingBuffer);
	#endif

	return 0;
}
#endif
