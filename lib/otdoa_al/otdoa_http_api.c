/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */
#include <string.h>
#include <stdlib.h>

#include <otdoa_al/otdoa_api.h>
#include <otdoa_al/otdoa_nordic_at.h>
#include <otdoa_al/otdoa_http_api.h>
#include "otdoa_http.h"
#include "otdoa_al_log.h"

LOG_MODULE_DECLARE(otdoa_al, LOG_LEVEL_INF);

extern void otdoa_http_register_callback(otdoa_api_callback_t cb);

/**
 * @brief Initialize the OTDOA AL library
 * @param[in] event_callback Callback function used by the library
 *                     to return results and status to the client
 * @return 0 on success
 */
int32_t otdoa_al_init(otdoa_api_callback_t event_callback)
{
	otdoa_http_register_callback(event_callback);
	otdoa_http_init();

#if CONFIG_LOG
	otdoa_log_init();
#endif /* CONFIG_LOG */

#if CONFIG_OTDOA_API_TLS_CERT_INSTALL
	bool exists;
	int rc = modem_key_mgmt_exists(CONFIG_OTDOA_TLS_SEC_TAG, OTDOA_TLS_CERT_TYPE, &exists);

	if (rc) {
		LOG_ERR("otdoa_al_init: failed to check for TLS certificate in tag %d: %d",
			      CONFIG_OTDOA_TLS_SEC_TAG, rc);
		return OTDOA_API_INTERNAL_ERROR;
	}

	if (!exists) {
		LOG_ERR("otdoa_al_init: TLS certificate not found in tag %d",
			      CONFIG_OTDOA_TLS_SEC_TAG);
		return OTDOA_API_INTERNAL_ERROR;
	}
#endif

	return OTDOA_API_SUCCESS;
}


#ifdef CONFIG_OTDOA_ENABLE_RESULTS_UPLOAD
int32_t otdoa_api_upload_results(const otdoa_api_results_t *p_results, const char *true_lat,
				 const char *true_lon, const char *notes)
{
	if (!p_results) {
		return OTDOA_API_ERROR_PARAM;
	}
	otdoa_api_results_t *p_http_results = calloc(1, sizeof(otdoa_api_results_t));

	if (!p_http_results) {
		return OTDOA_API_INTERNAL_ERROR;
	}
	memcpy(p_http_results, p_results, sizeof(otdoa_api_results_t));
	int rv = otdoa_http_send_results_upload(UPLOAD_SERVER_URL, p_http_results, notes, true_lat,
						true_lon);
	return (rv == 0 ? OTDOA_API_SUCCESS : OTDOA_API_INTERNAL_ERROR);
}
#endif
