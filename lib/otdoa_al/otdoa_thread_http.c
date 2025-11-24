/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#include <fcntl.h>
#include <modem/modem_info.h>
#include <modem/nrf_modem_lib.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>


#include "otdoa_al/otdoa_http_api.h"
#include "otdoa_al/otdoa_api.h"
#include "otdoa_al_log.h"
#include "otdoa_http.h"
#include "arpa/inet.h"

#define CHECK_IP

LOG_MODULE_DECLARE(otdoa_al, LOG_LEVEL_INF);

struct modem_param_info MPI = {0};
bool bModemInfoInit;

void http_modem_info_init(void)
{
	if (!bModemInfoInit) {
		modem_info_init();
		bModemInfoInit = true;
	}
}

#ifdef CHECK_IP
/**
 * Check if modem has a valid IP address
 *
 * @return true or false
 */
int is_ip_valid(char *szModemAddress, size_t szModemAddressLen)
{
	memset(szModemAddress, 0, szModemAddressLen);
	if (!bModemInfoInit) {
		modem_info_init();
		bModemInfoInit = true;
	}
	modem_info_string_get(MODEM_INFO_IP_ADDRESS, szModemAddress, szModemAddressLen);

	struct addrinfo addr;
	/**
	 * inet_pton returns 1 if the network address was successfully converted,
	 * 0 if not, and -1 if given an invalid address family
	 */
	return inet_pton(AF_INET, szModemAddress, &addr) == 1;
}
#endif

/**
 * Setup TLS options on a given socket
 *
 * @param fd Socket to set up
 * @param host Hostname to expect in TLS certificate
 * @return 0 on success, else error code
 */
int tls_setup(int fd, const char *host)
{
	int nErr = 0;

	/* security tag that we provisioned the certificate with */
	const sec_tag_t tls_sec_tag[] = {
		CONFIG_OTDOA_TLS_SEC_TAG,
	};

	/* set up TLS peer verification */
	enum {
		NONE,
		OPTIONAL,
		REQUIRED,
	};

	int verify;
#ifdef ACORN
	verify = OPTIONAL;
#else
	verify = OPTIONAL;
#endif

	nErr = setsockopt(fd, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
	if (nErr) {
		LOG_ERR("Failed to setup peer verification: %s", strerror(errno));
		return nErr;
	}

	/* associate the socket with the security tag we have provisioned the certificate with */
	nErr = setsockopt(fd, SOL_TLS, TLS_SEC_TAG_LIST, tls_sec_tag, sizeof(tls_sec_tag));
	if (nErr) {
		LOG_ERR("Failed to setup TLS sec tag: %s", strerror(errno));
		return nErr;
	}

	/* set the socket host */
	const char *server = host;

	if (!server) {
		server = otdoa_http_get_download_url();
	}
	LOG_INF("tls_setup(%d, %s)", fd, server);
	nErr = setsockopt(fd, SOL_TLS, TLS_HOSTNAME, server, strlen(server) + 1);
	if (nErr) {
		LOG_ERR("Failed to setup TLS Hostname: %s", strerror(errno));
		return nErr;
	}

	return nErr;
}

/**
 * Bind to server socket
 *
 * @param addr A struct addrinfo** pointing to the start of a connection list
 * @param pURL Pointer to URL string, null to use otdoa_http_get_download_url()
 * @param bDisableTls If HTTPS should be used to connect to the server
 * @param[out] pSeverAddress String buffer to write server IP address to
 * @param server_address_len Length of server address buffer
 * @return 0 for success, otherwise error value
 */
#define MAX_BIND_RETRIES 2 /* Retries take ~30 seconds so don't do too many!  (was 5) */
int otdoa_http_bind(struct addrinfo **res, const char *pURL, bool bDisableTls,
		    char *pServerAddress, size_t server_address_len)
{
	int rc;

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};

	/* always unbind first */
	otdoa_http_unbind(res);

	if (pURL == NULL) {
		pURL = otdoa_http_get_download_url();
	}

	LOG_INF("Binding to server [%s]", pURL);

	/* wait for successful bind */
	int nRetry;

	for (nRetry = 0; nRetry < MAX_BIND_RETRIES; nRetry++) {
		LOG_DBG("Trying getaddrinfo() %d", nRetry);
		rc = getaddrinfo(pURL, NULL, &hints, res);
		if (rc == 0) {
			break;
		}
		k_sleep(K_SECONDS(1));
	}
	if (nRetry >= MAX_BIND_RETRIES) {
		LOG_ERR("getaddrinfo() retry %d failed, err: %s", nRetry,
			      rc == EAI_SYSTEM ? strerror(errno) : gai_strerror(rc));
		return -1;
	}

	if (!inet_ntop(AF_INET, &((struct sockaddr_in *)(*res)->ai_addr)->sin_addr,
		       pServerAddress, server_address_len)) {
		LOG_ERR("Failed to convert address to text form: %d %s",
			      errno, strerror(errno));
		return -1;
	}
	LOG_INF("Server IP: %s", pServerAddress);

	if (bDisableTls) {
		((struct sockaddr_in *)(*res)->ai_addr)->sin_port = htons(HTTP_PORT);
	} else {
		((struct sockaddr_in *)(*res)->ai_addr)->sin_port = htons(HTTPS_PORT);
	}
	return 0;
}

/**
 * Unbind from server socket
 *
 * @param res Pointer to addrinfo to unbind
 */
int otdoa_http_unbind(struct addrinfo **res)
{
	if (res) {
		LOG_INF("http_unbind()");
		freeaddrinfo(*res);
		*res = NULL;
	}

	return 0;
}

/**
 * Open a secure connection to the server
 *
 * @param pG pointer to gHTTP containing settings
 * @param tls_host Hostname to configure if TLS is used
 * @return 0 for success, -1 for socket open failure, -2 for tls_setup failure, -3 for connect
 * failure
 */
int otdoa_http_connect(int *fdSocket, struct sockaddr *res, char *szModemAddress,
	size_t szModemAddressLen, const char *tls_host)
{
	int nErr = 0;
	bool bFound = false;
	int proto = tls_host ? IPPROTO_TLS_1_2 : IPPROTO_TCP;

	if (!fdSocket) {
		LOG_ERR("fdSocket is NULL");
		return -1;
	}

	LOG_INF("HTTP connect on protocol %d", proto);
	*fdSocket = socket(AF_INET, SOCK_STREAM, proto);
	if (*fdSocket == -1) {
		LOG_ERR("failed to open socket");
		return -1;
	}

#ifdef CHECK_IP /* check if our IP is set */
	for (int nRetry = 0; nRetry < 10; nRetry++) {
		if (is_ip_valid(szModemAddress, szModemAddressLen)) {
			bFound = true;
			break;
		}
		LOG_DBG("Waiting for IP address... %s", szModemAddress);
		k_sleep(K_SECONDS(1));
	}

	if (bFound) {
		LOG_DBG("nrf9161 IP address %s", szModemAddress);
	} else {
		LOG_ERR("failed to get IP address\r\n");
		return -1;
	}
#endif

	/* setup TLS socket options */
	if (!tls_host) {
		LOG_WRN("Skipping TLS");
	} else {
		nErr = tls_setup(*fdSocket, tls_host);
		if (nErr) {
			otdoa_http_disconnect(fdSocket);
			LOG_ERR("tls_setup error %s", strerror(errno));
			return -2;
		}
	}
	/* connect */
	LOG_DBG("connect() on socket %d", *fdSocket);
	nErr = connect(*fdSocket, res, sizeof(struct sockaddr_in));
	if (nErr) {
		LOG_ERR("connect failed: nErr = %d, %d -> %s", nErr, errno, strerror(errno));
		LOG_ERR("connect failed: fdSocket = %d, ai_addr = %p", *fdSocket, (void *)res);
		otdoa_http_disconnect(fdSocket);
		return -3;
	}
	return nErr;
}

/**
 * Disconnect from the server
 *
 * @param pG Pointer to gHTTP containing socket info
 * @return 0 on success, -1 on failure
 */
int otdoa_http_disconnect(int *fdSocket)
{
	int nReturn = -1;

	if (*fdSocket >= 0) {
		LOG_DBG("closing socket %d", *fdSocket);
		nReturn = close(*fdSocket);
		*fdSocket = -1;
	}
	return nReturn;
}

/**
 * Set a socket as blocking or nonblocking
 *
 * @param fd Socket to set
 * @param blocking True for blocking, false for nonblocking
 * @return true on success, otherwise false
 */
bool otdoa_http_set_sock_blocking(int fd, bool blocking)
{
	if (fd < 0) {
		return false;
	}

	/**
	 * Use zsock_fcntl() as described here:
	 *   https://github.com/zephyrproject-rtos/zephyr/issues/54347
	 */
	int flags = zsock_fcntl(fd, F_GETFL, 0);

	if (flags == -1) {
		LOG_ERR("fcntl() returned %d. errno = %d", flags, errno);
		return false;
	}
	flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
	bool rv = (zsock_fcntl(fd, F_SETFL, flags) == 0) ? true : false;

	if (!rv) {
		LOG_ERR("failed fctrl().  errno = %d", errno);
	}
	return rv;
}

/* wrappers */
ssize_t otdoa_http_recv(int socket, void *buffer, size_t length, int flags)
{
	return recv(socket, buffer, length, flags);
}
ssize_t otdoa_http_send(int socket, const void *buffer, size_t length, int flags)
{
	return send(socket, buffer, length, flags);
}

int otdoa_http_errno(void)
{
	return errno;
}

void otdoa_http_sleep(int msec)
{
	k_sleep(K_MSEC(msec));
}

int32_t otdoa_http_uptime(void)
{
	return k_uptime_get_32();
}

#if CONFIG_OTDOA_API_TLS_CERT_INSTALL
/**
 * Provision a TLS certificate to the modem
 *
 * @param[in] cert PEM-formatted TLS certificate to install for server
 * @param[in] len Length of the PEM certificate
 * @return 0 on success, else error code
 */
int otdoa_api_install_tls_cert(const char *tls_cert, const size_t cert_len)
{
	if (!tls_cert) {
		return OTDOA_API_ERROR_PARAM;
	}

	bool exists;
	int rc = modem_key_mgmt_exists(CONFIG_OTDOA_TLS_SEC_TAG, OTDOA_TLS_CERT_TYPE, &exists);

	if (rc) {
		LOG_ERR("Failed to check for certificate: %d", rc);
		return OTDOA_API_INTERNAL_ERROR;
	}

	if (exists) {
		/* for the sake of simplicity, we delete what is provisioned with
		 * our security tag and reprovision our certificate
		 */
		rc = modem_key_mgmt_delete(CONFIG_OTDOA_TLS_SEC_TAG, OTDOA_TLS_CERT_TYPE);
		if (rc) {
			LOG_WRN("Failed to delete existing certificate: %d", rc);
		}
	}

	/* provision certificate to the modem */
	LOG_DBG("Provisioning certificate");
	rc = modem_key_mgmt_write(CONFIG_OTDOA_TLS_SEC_TAG, OTDOA_TLS_CERT_TYPE, tls_cert,
				  cert_len);
	if (rc) {
		LOG_ERR("Failed to provision certificate: %d", rc);
		return OTDOA_API_INTERNAL_ERROR;
	}

	return OTDOA_API_SUCCESS;
}
#endif
