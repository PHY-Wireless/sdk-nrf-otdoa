// File: otdoa_thread.c

// #define DEBUG
#define TIMING

#include <zephyr/kernel.h>
#include <zephyr/posix/unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Time...
#include "date_time.h"

#include <otdoa_al/phywi_otdoa2al_api.h>
#include <otdoa_al/phywi_al2otdoa_api.h>
#include <otdoa_al/otdoa_http.h>

typedef struct message_work {
    struct k_work work;
    tOTDOA_HTTP_MESSAGE msg;
} tOTDOA_WORK_ITEM;

typedef struct stop_work {
    struct k_work work;
    uint32_t cancel_or_timeout;
} tOTDOA_STOP_WORK_ITEM;

static tOTDOA_STOP_WORK_ITEM rs_stop_work;

// Maximum allowable message size (library or AL)
static size_t max_msg_size = 0;

// prep the OTDOA and AL work queues
K_THREAD_STACK_DEFINE(http_workq_stack, CONFIG_OTDOA_HTTP_QUEUE_STACK_SIZE);
K_THREAD_STACK_DEFINE(rs_workq_stack, CONFIG_OTDOA_RS_QUEUE_STACK_SIZE);
struct k_work_q http_workq, rs_workq;

#define SLAB_COUNT 10
#define SLAB_ALIGN 4
// #define OTDOA_SLAB_MESSAGE_SIZE (sizeof (tOTDOA_WORK_ITEM))
struct k_mem_slab message_slab; // PHYW-399 ToDo: make static
static void* p_message_slab_buffer = NULL;

LOG_MODULE_DECLARE(otdoa, LOG_LEVEL_DBG);

int otdoa_message_free(void* msg) {
    if (msg != &rs_stop_work) {
        k_mem_slab_free(&message_slab, msg);
    }
    return 0;
}

void* otdoa_message_alloc(size_t length) {
    if (length > message_slab.info.block_size) {
        LOG_ERR("Message too large (%u bytes) for allocation", length);
        return NULL;
    }

    void* pReturn = 0;
    const int rv = k_mem_slab_alloc(&message_slab, (void**) &pReturn, K_NO_WAIT);
    if (rv == 0) {
        memset(pReturn, 0, length);
    } else {
        LOG_ERR("Memory allocation failure %d", rv);
    }
    return pReturn;
}

int otdoa_start() {
    // init the work queues
    struct k_work_queue_config http_cfg = {.name = "http_workq"};
    struct k_work_queue_config rs_cfg = {.name = "rs_workq"};
    k_work_queue_init(&http_workq);
    k_work_queue_init(&rs_workq);
    k_work_queue_start(&http_workq, http_workq_stack, K_THREAD_STACK_SIZEOF(http_workq_stack), CONFIG_OTDOA_HTTP_QUEUE_PRIORITY, &http_cfg);
    k_work_queue_start(&rs_workq, rs_workq_stack, K_THREAD_STACK_SIZEOF(rs_workq_stack), CONFIG_OTDOA_RS_QUEUE_PRIORITY, &rs_cfg);

    // Allocate message slab with buffers sized to fit maximum message size
    max_msg_size = OTDOA_MAX_MESSAGE_SIZE;       // Max size of OTDOA library messages
    if (sizeof(tOTDOA_HTTP_MESSAGE) > max_msg_size)
        max_msg_size = sizeof(tOTDOA_HTTP_MESSAGE);

    // Add the k_work structure to the size,
    // and round up to 32b words to account for any packing alignment issues
    max_msg_size += sizeof(struct k_work);
    max_msg_size = (max_msg_size + 3) & ~0x03;
    LOG_INF("Max message size = %u", max_msg_size);

    p_message_slab_buffer = calloc(SLAB_COUNT, max_msg_size);
    if (!p_message_slab_buffer) {
        LOG_ERR("Failed to allocate OTDOA message slab");
        return -1;
    }

    k_mem_slab_init(&message_slab, p_message_slab_buffer, max_msg_size, SLAB_COUNT);

    return 0;
}

int otdoa_stop() {
    // plug the queue, stopping more messages being added, and wait for it to empty
    if (k_work_queue_drain(&http_workq, true)) {
        LOG_ERR("Failed to drain OTDOA queue");
    }
    if (k_work_queue_drain(&rs_workq, true)) {
        LOG_ERR("Failed to drain AL queue");
    }

    if (p_message_slab_buffer) {
        free(p_message_slab_buffer);
        p_message_slab_buffer = NULL;
    }
    max_msg_size = 0;

    return 0;
}

void otdoa_queue_handle_rs(struct k_work* work) {
    if (!work) return;
    struct message_work* parent = CONTAINER_OF(work, struct message_work, work);
    if (parent) {
        otdoa_rs_handle_msg((void*) &parent->msg);
        otdoa_message_free(parent);
    }
}

void otdoa_queue_handle_stop(struct k_work* work) {
    if (!work) return;
    tOTDOA_STOP_WORK_ITEM* parent = CONTAINER_OF(work, tOTDOA_STOP_WORK_ITEM, work);
    if (parent) {
        otdoa_rs_handle_stop(parent->cancel_or_timeout);
    }
}

void otdoa_queue_handle_http(struct k_work* work) {
    if (!work) return;
    struct message_work* parent = CONTAINER_OF(work, struct message_work, work);
    if (parent) {
        otdoa_http_handle_message((tOTDOA_HTTP_MESSAGE*) &parent->msg);
        otdoa_message_free(parent);
    }
}

int otdoa_queue_http_message(const void* pv_msg, const size_t length) {
    const tOTDOA_HTTP_MESSAGE* msg = (const tOTDOA_HTTP_MESSAGE*) pv_msg;

    if (length > max_msg_size) {
        LOG_ERR("otdoa_queue_http_message: message too long (%u bytes)", length);
        return -1;
    }

    tOTDOA_WORK_ITEM* work = otdoa_message_alloc(length);
    if (!work) {
        LOG_ERR("otdoa_queue_http_message: failed to allocate message");
        return -1;
    }

    k_work_init(&work->work, otdoa_queue_handle_http);
    memcpy(&work->msg, msg, length);

    const int rc = k_work_submit_to_queue(&http_workq, &work->work);
    if (rc != 1) {
        // 0 and 2 are success values, but imply the work item has been double-allocated
        LOG_ERR("otdoa_queue_http_message: failed to send message: %d", rc);
        otdoa_message_free(work);
        return -1;
    }

    return 0;
}

int otdoa_queue_rs_message(const void* pv_msg, const size_t length) {
    tOTDOA_WORK_ITEM* work;

    if (length > max_msg_size) {
        LOG_ERR("otdoa_queue_rs_message: message too long (%u bytes)", length);
        return -1;
    }

    work = otdoa_message_alloc(length);
    if (!work) {
        LOG_ERR("otdoa_queue_rs_message: failed to allocate message");
        return -1;
    }

    k_work_init(&work->work, otdoa_queue_handle_rs);
    memcpy(&work->msg, pv_msg, length);

    const int rc = k_work_submit_to_queue(&rs_workq, &work->work);
    if (rc != 1) {
        // 0 and 2 are success values, but imply the work item has been double-allocated
        LOG_ERR("otdoa_queue_rs_message: failed to send message: %d", rc);
        otdoa_message_free(work);
        return -1;
    }

    return 0;
}

int otdoa_queue_stop_request(uint32_t cancel_or_timeout) {

    // use a known static work item so we can 'peek' and see if a stop request is queued
    rs_stop_work.cancel_or_timeout = cancel_or_timeout;

    k_work_init(&rs_stop_work.work, otdoa_queue_handle_stop);

    const int rc = k_work_submit_to_queue(&rs_workq, &rs_stop_work.work);
    if (rc != 1) {
        // 0 and 2 are success values, but imply the work item has been double-allocated
        LOG_ERR("otdoa_queue_rs_message: failed to send stop req");
        return -1;
    }

    return 0;
}

int otdoa_message_check_pending_stop() {
    const int pending = k_work_is_pending(&rs_stop_work.work);
    if (pending) k_work_cancel(&rs_stop_work.work);
    return !!pending;
}
