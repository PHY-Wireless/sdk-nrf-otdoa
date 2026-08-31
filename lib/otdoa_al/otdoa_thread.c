/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#include <otdoa_al/otdoa_al2otdoa_api.h>
#include <otdoa_al/otdoa_otdoa2al_api.h>
#include <otdoa_al/otdoa_http_api.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/posix/unistd.h>

#include "otdoa_al_log.h"
#include "otdoa_http.h"

LOG_MODULE_REGISTER(otdoa_al, LOG_LEVEL_INF);

#define SLAB_COUNT (CONFIG_OTDOA_MESSAGE_SLAB_COUNT)

static struct k_fifo rs_fifo;
static struct k_thread rs_thread_data;
static void *message_slab_buffer;
struct k_mem_slab message_slab;

static struct {
	/* boolean termination control */
	atomic_t terminate;

	/* boolean ready state */
	atomic_t ready;

	/* integer stop-request pending state
	 * the FAIL and CANCEL constants passed into otdoa_queue_stop_request are nonzero,
	 * so a nonzero value stored here indicates a pending stop
	 */
	atomic_t http_stop_pending;
} gOTDOA;

K_THREAD_STACK_DEFINE(rs_thread_stack, CONFIG_OTDOA_RS_THREAD_STACK_SIZE);

void rs_entry_point(void *p1, void *p2, void *p3);

int otdoa_start(void)
{
	/* init the message slab */
	const unsigned int MAX_MSG_SIZE = MAX(OTDOA_MAX_MESSAGE_SIZE, OTDOA_HTTP_MAX_MSG_SIZE);

	message_slab_buffer = calloc(SLAB_COUNT, MAX_MSG_SIZE);
	k_mem_slab_init(&message_slab, message_slab_buffer, MAX_MSG_SIZE, SLAB_COUNT);

	/* init the RS fifo thread */
	k_fifo_init(&rs_fifo);
	k_thread_create(&rs_thread_data, rs_thread_stack, K_THREAD_STACK_SIZEOF(rs_thread_stack),
			rs_entry_point, NULL, NULL, NULL, CONFIG_OTDOA_RS_THREAD_PRIORITY,
			K_FP_REGS, K_NO_WAIT);
	k_thread_name_set(&rs_thread_data, "otdoa_thread");

	return 0;
}

int otdoa_stop(void)
{
	atomic_set(&gOTDOA.terminate, 1);
	int rc = k_thread_join(&rs_thread_data, K_MSEC(10000));

	if (rc) {
		LOG_ERR("Timed out waiting for RS Thread to terminate: %d", rc);
	}

	free(message_slab_buffer);
	return rc;
}

void *otdoa_message_alloc(size_t length)
{
	if (length > message_slab.info.block_size) {
		LOG_ERR("message too large for allocation (%u bytes)", length);
		return NULL;
	}

	void *alloc;
	int rc = k_mem_slab_alloc(&message_slab, &alloc, K_NO_WAIT);

	if (rc) {
		LOG_ERR("Memory allocation failure: %d", rc);
		return NULL;
	}

	return alloc;
}

int otdoa_message_free(void *msg)
{
	k_mem_slab_free(&message_slab, msg);
	return 0;
}

void rs_entry_point(void *p1, void *p2, void *p3)
{
	LOG_INF("RS Thread Started");

	while (!atomic_get(&gOTDOA.terminate)) {
		void *msg = k_fifo_get(&rs_fifo, K_FOREVER);

		if (!msg) {
			LOG_INF("RS Thread timed out with no message");
			continue;
		}

		otdoa_handle_message(msg);
		otdoa_message_free(msg);
	}

	LOG_INF("RS Thread Exit");
	atomic_clear(&gOTDOA.terminate);
}

int otdoa_queue_http_message(const void *msg, const size_t length)
{
	return otdoa_queue_rs_message(msg, length);
}

int otdoa_queue_rs_message(const void *pv_msg, const size_t length)
{
	if (!pv_msg) {
		LOG_ERR("no message");
		return -1;
	}

	void *msg = otdoa_message_alloc(length);

	if (!msg) {
		LOG_ERR("failed to allocate rs message");
		return -1;
	}

	memcpy(msg, pv_msg, length);
	k_fifo_put(&rs_fifo, msg);

	return 0;
}

int otdoa_message_check_pending_stop(void)
{
	/* we always want to reset the stop state when we check it */
	return atomic_clear(&gOTDOA.http_stop_pending);
}

int32_t otdoa_queue_stop_request(int fail_or_cancel)
{
	if (0 != atomic_set(&gOTDOA.http_stop_pending, fail_or_cancel)) {
		/* there was a non-zero value already pending, so we've "double-stopped" */
		LOG_WRN("A stop has already been requested, overwriting previous request");
		return OTDOA_API_INTERNAL_ERROR;
	}

	return 0;
}
