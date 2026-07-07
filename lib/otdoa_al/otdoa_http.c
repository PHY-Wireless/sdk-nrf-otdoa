/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <otdoa_al/otdoa_fs.h>
#include <otdoa_al/otdoa_api.h>
#include <otdoa_al/otdoa_otdoa2al_api.h>
#include <otdoa_al/otdoa_http_api.h>

#include "otdoa_http.h"
#include "otdoa_al_log.h"

LOG_MODULE_DECLARE(otdoa_al, LOG_LEVEL_INF);

/* forward declarations */
#ifdef OTDOA_ENABLE_BSA_CIPHER
static int encrypt_ubsa = 1;
#else
static int encrypt_ubsa;
#endif
static int compress_ubsa = 1;

void http_set_ubsa_params(int enc, int comp)
{
	encrypt_ubsa = enc;
	compress_ubsa = comp;
}

/* functions inside nordic code */
extern ssize_t http_send(int socket, const void *buffer, size_t length, int flags);

/* otdoa lib functions */
extern void otdoa_http_h1_init(void);
extern int otdoa_http_h1_rebind(const char *url);
extern int otdoa_http_h1_unbind(void);
extern int otdoa_http_h1_handle_message(void *pMsg);

/**
 * Set default values in gHTTP
 *
 * @param gHTTP Pointer to tOTDOA_HTTP_MEMBERS struct to initialize
 */
void otdoa_http_defaults(void)
{
	otdoa_http_h1_init();
}

/**
 * Send a specified number of bytes of the request to the server
 *
 * @param fdSocket Socket to send on
 * @param request Pointer to request to send
 * @param n Size of request in bytes
 ( @param nOff Pointer to offset of data in buffer
 * @return 0 on success, else -1
 */
int otdoa_http_send_request(int fdSocket, const char *request, const size_t n, size_t *nOff)
{
	int bytes;

	do {
		bytes = otdoa_http_send(fdSocket, &request[*nOff], n - *nOff, 0);
		if (bytes < 0) {
			LOG_ERR("http_send_request: send failed: %s", strerror(errno));
			return -1;
		}
		*nOff += bytes;
	} while (*nOff < strlen(request));
	return 0;
}

/**
 * Open a file and write data to it
 *
 * @param path Path to write data to
 * @param data Data to write to file
 * @param len Length of data to write
 * @return Positive number of bytes written to file, otherwise negative error code
 */
int http_write_to_file(const char *path, void *data, size_t len)
{
	LOG_DBG("Attempting to write %zu bytes from %p to %s", len, data, path);
	int rc = 0;
	tOFS_FILE *fp = FOPEN(path, "w");

	if (!fp) {
		return -errno;
	}

	rc = FWRITE(data, len, 1, fp);
	if (rc != len) {
		rc = -1;
	}

	FCLOSE(fp);

	return rc;
}

/**
 * HTTP FSM top-level message handling
 * Replaces http_entry_point from nordic/otdoa_thread_http.c
 *
 * @param pMsg Pointer to message to handle
 * @return 0 on success, otherwise negative error code
 */
int otdoa_http_handle_message(void *pMsg)
{
	return otdoa_http_h1_handle_message(pMsg);
}

int otdoa_http_send_message(void *pMsg, uint32_t u32Len)
{
	return otdoa_queue_http_message(pMsg, u32Len);
}

/**
 * Perform HTTP setup
 *
 * @return 0 on success, otherwise negative error code
 */
void otdoa_http_init(void)
{
	otdoa_http_h1_init();
}

static char szDownloadURL[51] = {0};
void otdoa_http_set_download_url(const char *const pszURL)
{
	if (pszURL == NULL) {
		memset(szDownloadURL, 0, sizeof(szDownloadURL));
	} else {
		strncpy(szDownloadURL, pszURL, sizeof(szDownloadURL) - 1);
	}
}

const char *otdoa_http_get_download_url(void)
{
	if (szDownloadURL[0] != '\0') {
		return szDownloadURL;
	} else {
		return DEFAULT_BSA_DL_SERVER_URL;
	}
}

const char *otdoa_http_get_upload_pw(void)
{
	return RESULTS_UPLOAD_PW;
}

static char szResultsUploadURL[51] = {0};
void otdoa_http_set_results_upload_url(const char *const pszURL)
{
	if (pszURL == NULL) {
		memset(szResultsUploadURL, 0, sizeof(szResultsUploadURL));
	} else {
		strncpy(szResultsUploadURL, pszURL, sizeof(szResultsUploadURL) - 1);
	}
}

const char *otdoa_http_get_upload_url(void)
{
	if (szResultsUploadURL[0] != '\0') {
		return szResultsUploadURL;
	} else {
		return UPLOAD_SERVER_URL;
	}
}
