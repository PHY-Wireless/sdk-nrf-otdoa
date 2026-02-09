/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#ifndef OTDOA_HTTP_H
#define OTDOA_HTTP_H

#include <stdbool.h>
#include <otdoa_al/otdoa_api.h>
#include <modem/modem_key_mgmt.h>

/* collect our Kconfig values */
#define HTTPS_PORT	    CONFIG_OTDOA_HTTPS_PORT
#define HTTP_PORT	    CONFIG_OTDOA_HTTP_PORT

#define DEFAULT_BSA_DL_SERVER_URL "api.prod.hellaphy.cloud"

#define OTDOA_TLS_CERT_TYPE MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN

#ifdef CONFIG_OTDOA_ENABLE_RESULTS_UPLOAD
#define RESULTS_UPLOAD_PW CONFIG_OTDOA_RESULTS_UPLOAD_PW
#define UPLOAD_SERVER_URL CONFIG_OTDOA_UPLOAD_SERVER_URL
#endif

/* set to 1 to have HTTP functions use scratchpad, else use malloc()/free() */
#define HTTP_USE_SCRATCHPAD		0
#if HTTP_USE_SCRATCHPAD
#error "Need to resolve scratchpad use" /* See PHYW-496 */
#endif

/* decrease again to allow for the addition of ubsa-seed that server is now adding to response */
#if HTTP_USE_SCRATCHPAD
#define HTTP_RANGE_REQUEST_SIZE 6200 /* NB: HTTP req size can be larger than limit for HTTPS */
#define HTTP_BUF_SIZE		6700
#else
/* can use larger buffers because we dynamically allocate them */
#define HTTP_RANGE_REQUEST_SIZE 12000 /* NB: HTTP req size can be larger than limit for HTTPS */
#define HTTP_BUF_SIZE		12288
#endif
/* HTTPS buffer size is limited by Nordic TLS implementation */
#define HTTPS_RANGE_REQUEST_SIZE 1500
#define HTTPS_BUF_SIZE		 2048

#define HTTPS_RANGE_MAX_DEFAULT 100000

ssize_t otdoa_http_send(int fdSocket, const void *buffer, size_t length, int flags);

/* Public members */
void otdoa_http_init(void);
void otdoa_disable_tls(bool bDisableTLS);
void otdoa_skip_config_dl(bool bSkipConfigDL);
bool otdoa_get_skip_config_dl(void);
bool otdoa_get_disable_tls(void);
void otdoa_http_disable_encrypt(bool bDisableEncryption);
bool otdao_http_get_encryption_disable(void);
void otdoa_http_override_auth_resp(int override);
int otdoa_http_get_override_auth_resp(void);

int otdoa_http_send_log_upload(const char *vf, uint32_t repeat);
int otdoa_http_send_vector_upload(const char *vf, uint32_t repeat);
int otdoa_http_send_results_upload(const char *pURL, otdoa_api_results_t *pResults,
				   const char *p_notes, const char *p_true_lat,
				   const char *p_true_lon);
int otdoa_http_send_test_jwt(void);
int otdoa_http_send_rebind_socket(void);

bool otdoa_http_check_pending_stop(void);
int cert_provision(const char *cert, size_t len);

#endif /* OTDOA_HTTP_H */
