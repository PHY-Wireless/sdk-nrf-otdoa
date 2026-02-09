/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#ifndef OTDOA_HTTP_API_H
#define OTDOA_HTTP_API_H

#include <stdbool.h>

#include <nrf_socket.h>

#ifdef HOST
/* basic definitions needed for tests */
struct sockaddr;
struct addrinfo {
	struct sockaddr *ai_addr;
};

#include <stdint.h>
#include <stddef.h>
#else
#include <zephyr/posix/netdb.h>
#include <zephyr/net/socket.h>
#endif

/* Default DLEARFCN if not available from modem */
#define DEFAULT_UBSA_DLEARFCN 5230u
#define DEFAULT_UBSA_PCI      289u
#define UNKNOWN_UBSA_DLEARFCN UINT16_MAX
#define UNKNOWN_UBSA_PCI      UINT16_MAX

/* Maximum size for an HTTP message
 * This will not be the exact size of a message, but is guaranteed to be large enough to store one
 */
#define OTDOA_HTTP_MAX_MSG_SIZE 128

int otdoa_http_connect(int *fdSocket, const struct nrf_sockaddr *res, char *szModemAddress,
			size_t szModemAddressLen, const char *tls_host);
int otdoa_http_disconnect(int *fdSocket);
int otdoa_http_bind(struct nrf_addrinfo **res, const char *pURL, bool bDisableTls, char *pServerAddress,
			size_t server_address_len);
int otdoa_http_unbind(struct nrf_addrinfo **res);
int otdoa_http_rebind(const char *url);
int otdoa_http_send_request(int fdSocket, const char *request, const size_t n, size_t *nOff);
int otdoa_http_errno(void);
bool otdoa_http_set_sock_blocking(int fd, bool blocking);
void otdoa_http_sleep(int msec);
int32_t otdoa_http_uptime(void);

int otdoa_http_send_message(void *pMsg, uint32_t len);

const char *otdoa_http_get_download_url(void);
int http_write_to_file(const char *path, void *data, size_t len);

#ifdef CONFIG_OTDOA_ENABLE_RESULTS_UPLOAD
extern const char *otdoa_http_get_upload_pw(void);
#endif

#endif /* ifndef OTDOA_HTTP_API_H */
