/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <modem/location.h>
#include <otdoa_al/phywi_otdoa_api.h>

#include "location_core.h"
#include "otdoa_little_fs.h"

LOG_MODULE_DECLARE(location, CONFIG_LOCATION_LOG_LEVEL);

#define UBSA_FILE_PATH "/lfs/ubsa.csv"

struct method_otdoa_start_work_args {
	struct k_work work_item;
	const struct location_otdoa_config *otdoa_config;
};

static struct method_otdoa_start_work_args method_otdoa_start_work;
static bool running;

// TODO: This is needed by OTDOA library but should be handled differently in the end
void otdoa_shell_printf(void* psh, const char* pstring)
{
    LOG_DBG("%s", pstring);
}

void method_otdoa_event_handler(const otdoa_api_event_data_t *event_data)
{
	static uint8_t dl_in_session = 0;
	static struct location_data location_result = { 0 };

	LOG_DBG("method_otdoa_event_handler %d", event_data->event);

	switch (event_data->event) {
	case OTDOA_EVENT_RESULTS:
		LOG_INF("OTDOA_EVENT_RESULTS:");
		LOG_INF("  latitude: %.06f", event_data->results.latitude);
		LOG_INF("  longitude: %.06f", event_data->results.longitude);
		LOG_INF("  accuracy: %.01f m", (double)event_data->results.accuracy);
		//display_result_details(&event_data->results.details);
		LOG_INF("OTDOA position estimate SUCCESS");

		location_result.latitude = event_data->results.latitude;
		location_result.longitude = event_data->results.longitude;
		location_result.accuracy = event_data->results.accuracy;

		location_core_event_cb(&location_result);
		break;

	case OTDOA_EVENT_FAIL:
		LOG_ERR("OTDOA_EVENT_FAIL:  failure code = %d", event_data->failure_code);
		location_core_event_cb_error();
		break;

        case OTDOA_EVENT_UBSA_DL_REQ:
        {
            // OTDOA Library requests download of a new uBSA file
            dl_in_session = 1;
            // Call the API to start the DL
            LOG_INF("OTDOA_EVENT_UBSA_DL_REQ:");
            LOG_INF("  ecgi: %u  dlearfcn: %u",
	    		event_data->dl_request.ecgi,
			event_data->dl_request.dlearfcn);
            LOG_INF("  max cells: %u  radius: %u",
	    		event_data->dl_request.max_cells,
	    		event_data->dl_request.ubsa_radius_meters);
            int err = otdoa_api_ubsa_download(&event_data->dl_request, UBSA_FILE_PATH, false);
            if (err != OTDOA_API_SUCCESS)
            {
                LOG_ERR("otdoa_api_ubsa_download() failed with return %d", err);
                err = otdoa_api_cancel_session();
                if (err != OTDOA_API_SUCCESS)
                {
                    LOG_ERR("otdoa_api_cancel_session() failed with return %d", err);
                }
            }
            break;
        }

        case OTDOA_EVENT_UBSA_DL_COMPL:
        {
            LOG_INF("OTDOA_EVENT_UBSA_DL_COMPL:");
            LOG_INF("  status: %d", event_data->dl_compl.status);

            // If DL request was done as part of a session, indicate to the OTDOA library
            // that the updated uBSA file is now available.
            if (dl_in_session)
            {
                dl_in_session = 0;
                int err = otdoa_api_ubsa_available(event_data->dl_compl.status, UBSA_FILE_PATH);
                if (err != OTDOA_API_SUCCESS)
                {
                    LOG_ERR("otdoa_api_ubsa_available() failed with return %d", err);
                    err = otdoa_api_cancel_session();
                    if (err != OTDOA_API_SUCCESS)
                    {
                        LOG_ERR("otdoa_api_cancel_session() failed with return %d", err);
                    }
                }
            }
            else if (event_data->dl_compl.status == OTDOA_API_SUCCESS)
            {
                LOG_INF("OTDOA uBSA DL SUCCESS");
            }
            break;
        }

	default:
		LOG_WRN("Unhandle event %d in otdoa_event_handler()", event_data->event);
		location_core_event_cb_error();
		break;
	}
}

static void method_otdoa_positioning_work_fn(struct k_work *work)
{
	int err;
	struct method_otdoa_start_work_args *work_data =
		CONTAINER_OF(work, struct method_otdoa_start_work_args, work_item);
	const struct location_otdoa_config *otdoa_config = work_data->otdoa_config;

	// TODO: Same timeout going to library
	location_core_timer_start(otdoa_config->timeout);

	otdoa_api_session_params_t params = { 0 };
	params.session_length = otdoa_config->session_length;
	params.capture_flags = 0;
	params.timeout = otdoa_config->timeout;

	err = otdoa_api_start_session(&params, method_otdoa_event_handler);
	if (err != OTDOA_API_SUCCESS) {
		LOG_ERR("otdoa_api_start_session() failed with return %d", err);
	}
	LOG_DBG("otdoa_api_start_session() %d", err);
}

int method_otdoa_cancel(void)
{
	int err;

	if (running) {
		err = otdoa_api_cancel_session();
		if (err != OTDOA_API_SUCCESS) {
			LOG_ERR("otdoa_api_cancel_session() failed with return %d", err);
		}
		LOG_DBG("otdoa_api_cancel_session() %d", err);

		(void)k_work_cancel(&method_otdoa_start_work.work_item);

		running = false;
	} else {
		return -EPERM;
	}
	return 0;
}

int method_otdoa_location_get(const struct location_request_info *request)
{
	k_work_init(
		&method_otdoa_start_work.work_item,
		method_otdoa_positioning_work_fn);

	method_otdoa_start_work.otdoa_config = request->otdoa;

	k_work_submit_to_queue(
		location_core_work_queue_get(),
		&method_otdoa_start_work.work_item);

	running = true;

	return 0;
}

int method_otdoa_init(void)
{
	LOG_DBG("otdoa_api_init()");
	running = false;

	otdoa_api_cfg_set_file_path("/lfs/config");

	int err = otdoa_api_init(UBSA_FILE_PATH, method_otdoa_event_handler);
	if (err != 0) {
		LOG_ERR("otdoa_api_init() failed with return %d", err);
		return err;
	}

	return 0;
}
