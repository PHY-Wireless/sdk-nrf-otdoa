/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#ifndef OTDOA_HTTP_API_H
#define OTDOA_HTTP_API_H

#include <stdbool.h>

#ifdef HOST
#include <sys/socket.h>
#include <netdb.h>
#else
#include <zephyr/net/socket.h>
#include <zephyr/posix/netdb.h>
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

int otdoa_http_connect(int* fdSocket, struct sockaddr* res, char* szModemAddress,
        size_t szModemAddressLen, const char *tls_host);
int otdoa_http_disconnect(int* sockfd);
int otdoa_http_bind(struct addrinfo **res, const char *pURL, bool bDisableTls,
                    char *pServerAddress, size_t server_address_len);
int otdoa_http_unbind(struct addrinfo *res);
int otdoa_http_send_request(int fdSocket, const char *request, const size_t n, size_t *nOff);

int otdoa_http_send_message(void *pMsg, uint32_t len);
void otdoa_http_invoke_callback_dl_compl(int status);
void otdoa_http_invoke_callback_ul_compl(int status);

const char *otdoa_http_get_download_url(void);
int http_write_to_file(const char *path, void *data, size_t len);

#endif /* ifndef OTDOA_HTTP_API_H */
