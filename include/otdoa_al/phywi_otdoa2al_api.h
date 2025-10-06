/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

 /**
  * @note API is designed to be accessed by the OTDOA libary.  In general,
  * it should not be used by the user application code.
  */

#ifndef PHYWI_OTDOA2AL_API__
#define PHYWI_OTDOA2AL_API__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "otdoa_nordic_at_h1.h"
#include "phywi_otdoa_api.h"

int otdoa_start();                                                                           \
int otdoa_stop();                                                                            \

 /**
 * @brief Initialize the OTDOA AL library
 * @param[in] event_callback Callback function used by the library
 *                     to return results and status to the client
 * @return 0 on success
 */
int32_t otdoa_al_init(otdoa_api_callback_t event_callback);

/***************************************************************************
 * Message Handling Functions
 */
int32_t otdoa_send_to_main_thread(const void* pvMsg, uint32_t u32Length);
int otdoa_queue_rs_message(const void* msg, const size_t length);
int otdoa_queue_http_message(const void* msg, const size_t length);
int32_t otdoa_http_send_stop_req(int cancel_or_timeout);
int otdoa_rs_send_stop_req(uint32_t cancel_or_timeout);
int otdoa_message_check_pending_stop();
int otdoa_message_free(void*  pMessage);

/***************************************************************************
 * Timer Handling Functions
 */

int otdoa_timers_init();
int otdoa_timer_start(unsigned uTimerNo, unsigned uDurationMS);
int otdoa_timer_stop(unsigned uTimerNo);
int otdoa_timer_active(unsigned uTimerNo);

/***************************************************************************
 * Log Functions
 */
unsigned otdoa_log_level_set(int level, const char *backend);
void otdoa_log_err(const char *fmt, ...);
void otdoa_log_wrn(const char *fmt, ...);
void otdoa_log_inf(const char *fmt, ...);
void otdoa_log_dbg(const char *fmt, ...);
void otdoa_log_hexdump_inf(void* data, unsigned length, char* str);

/***************************************************************************
 * PRS Sample Buffering Functions
 *  Alloc/free of  sample buffers
 */
void *otdoa_alloc_samples(size_t uLen);
int otdoa_free_samples(void *pBuffer);

/***************************************************************************
 * Misc. Functions
 */
void otdoa_sleep_msec(int msec);

#ifdef __cplusplus
}
#endif
#endif  // PHYWI_OTDOA2AL_API__
