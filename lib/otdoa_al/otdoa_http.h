/*------------------------------------------------------------------+
 *     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
 *       Copyright (C) Acorn Technologies, Inc.  2015-2025          |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdoa_http.h
 *
 * OTDOA HTTP Module
 *
 */

#ifndef OTDOA_HTTP_H
#define OTDOA_HTTP_H

#include <stdbool.h>

#include <otdoa_al/otdoa_log.h>
#include <otdoa_al/phywi_otdoa_api.h>
#include "autoconf.h"
#include "modem/modem_key_mgmt.h"

#define DEFAULT_UBSA_DLEARFCN   5230    // Default DLEARFCN if not available from modem

// collect our Kconfig values
#define BSA_DL_SERVER_URL   CONFIG_OTDOA_BSA_DL_SERVER_URL
#define HTTPS_PORT          CONFIG_OTDOA_HTTPS_PORT
#define HTTP_PORT           CONFIG_OTDOA_HTTP_PORT
#define CFG_DL_INTERVAL     CONFIG_OTDOA_CFG_DL_INTERVAL
#define OTDOA_TLS_CERT_TYPE MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN

#ifdef CONFIG_OTDOA_ENABLE_RESULTS_UPLOAD
#   define RESULTS_UPLOAD_PW  CONFIG_OTDOA_RESULTS_UPLOAD_PW
#   define UPLOAD_SERVER_URL  CONFIG_OTDOA_UPLOAD_SERVER_URL
#endif

#define HTTP_USE_SCRATCHPAD 0       // set to 1 to have HTTP functions use scratchpad, else use malloc()/free()
#if HTTP_USE_SCRATCHPAD
    #error "Need to resolve scratchpad use" // See PHYW-496
#endif


#define HELLAPHY_IP_ADDR_LMAX 20

// decrease again to allow for the addition of ubsa-seed that server is now adding to response
#if HTTP_USE_SCRATCHPAD
    #define HTTP_RANGE_REQUEST_SIZE 6200    // NB: HTTP req size can be larger than limit for HTTPS
    #define HTTP_BUF_SIZE           6700
#else
    // can use larger buffers because we dynamically allocate them
    #define HTTP_RANGE_REQUEST_SIZE 12000   // NB: HTTP req size can be larger than limit for HTTPS
    #define HTTP_BUF_SIZE           12288
#endif
// HTTPS buffer size is limited by Nordic TLS implementation
#define HTTPS_RANGE_REQUEST_SIZE 1500
#define HTTPS_BUF_SIZE 2048

#define HTTPS_RANGE_MAX_DEFAULT 100000

// New response header field lengths for HTTP H1
// #define IV_LMAX (32 + 1) // @note @dhe this might only be 16 bytes
#define IV_LMAX     (16)    // Binary IV size
#define UBSA_TOKEN_LMAX (64 + 1)

// pubkey is stored in binary DER format.  Since we are
// not doing all the ASN.1 decoding of this format, we require
// that the format be constant, and we select the public key
// from a fixed offset with the DER format
#define PUBKEY_DER_LEN      91          // Length of the DER-format binary
// PHYW-484 #define PUBKEY_LMAX (JWT_PUBKEY_SIZE)   // binary pubkey size
#define PUBKEY_LMAX         (65)        // binary pubkey size
#define PUBKEY_DER_OFFSET   26          // offset of the actual key, binary format

// Serving Cell ECGI blacklist to block ubsa dl attempts
#define BLACKLIST_SIZE (5)      // Length of blacklist to keep
// Number of requests before blacklist entry expires
#define BLACKLIST_TIMEOUT (CONFIG_OTDOA_HTTP_BLACKLIST_TIMEOUT)

// This macro provides for optional fields such as linked list pointers or reference counts.
// Zephyr uses this field to make a linked list that forms the FIFO.  So messages that
// are sent through Zephyr FIFOs (as opposed to e.g. workqueues) need this field.  Currently
// that includes messages sent to the capture thread.
#define RESERVED void *fifo_reserved;
#define HTTPS_URL_LMAX 32

typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
} tOTDOA_MSG_HTTP_HEADER;

// message for http module to download uBSA
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
    char pURL[HTTPS_URL_LMAX];
    bool bResetBlacklist;
    unsigned uEcgi;
    unsigned uDlearfcn;
    unsigned uRadius;
    unsigned uNumCells;
    unsigned uNumRepeatCount;
} tOTDOA_MSG_HTTP_GET_UBSA;

// message for http module to download config file
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
} tOTDOA_MSG_HTTP_GET_CFG;

// message for http module to upload the vector
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
    char vf[HTTPS_URL_LMAX];
    unsigned uNumRepeatCount;
} tOTDOA_MSG_HTTP_UPLOAD_VECTOR;

// message for http module to upload log files
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
    char vf[HTTPS_URL_LMAX];
    unsigned uNumRepeatCount;
} tOTDOA_MSG_HTTP_UPLOAD_LOG;

// message for http module to upload otdoa results
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
    char pURL[HTTPS_URL_LMAX];
    otdoa_api_results_t *pResults;
    const char* p_notes;
    const char* p_true_lat;
    const char* p_true_lon;
} tOTDOA_MSG_HTTP_UPLOAD_RESULTS;

// message for http module to test the JWT generation
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
} tOTDOA_MSG_HTTP_TEST_JWT;

// message for http module to rebind the socket
typedef struct {
    RESERVED;
    uint32_t u32MsgId;
    uint32_t u32MsgLen;
} tOTDOA_MSG_HTTP_REBIND;

// OTDOA HTTP Message ID and name definitions
#ifndef GENERATE_ENUM
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#endif

#define FOREACH_OTDOA_HTTP_MSG(__MSG)    \
        __MSG(OTDOA_HTTP_MSG_EVENT)   \
        __MSG(OTDOA_HTTP_MSG_GET_H1_UBSA) \
        __MSG(OTDOA_HTTP_MSG_UPLOAD_OTDOA_RESULTS) \
        __MSG(OTDOA_HTTP_MSG_UPLOAD_VECTOR_FILE) \
        __MSG(OTDOA_HTTP_MSG_UPLOAD_LOG_FILE) \
        __MSG(OTDOA_HTTP_MSG_REBIND_SOCKET) \
        __MSG(OTDOA_HTTP_MSG_TEST_JWT) \
        __MSG(OTDOA_HTTP_MSG_GET_H1_CONFIG_FILE) \

enum {
    FOREACH_OTDOA_HTTP_MSG(GENERATE_ENUM)
};

typedef struct OTDOA_HTTP_BLACKLIST {
    unsigned ecgi;
    int age;
} tOTDOA_HTTP_BLACKLIST;

//typedef tPXL_MSG_UNION tOTDOA_HTTP_MESSAGE;
typedef union {
    tOTDOA_MSG_HTTP_HEADER header;
    tOTDOA_MSG_HTTP_GET_UBSA http_get_ubsa;
    tOTDOA_MSG_HTTP_GET_CFG http_get_cfg;
    tOTDOA_MSG_HTTP_UPLOAD_VECTOR http_upload_vector;
    tOTDOA_MSG_HTTP_UPLOAD_LOG http_upload_log;
    tOTDOA_MSG_HTTP_UPLOAD_RESULTS http_upload_results;
    tOTDOA_MSG_HTTP_REBIND http_rebind;
    tOTDOA_MSG_HTTP_TEST_JWT http_test_jwt;
} tOTDOA_HTTP_MESSAGE;

//	HTTPS Thread Members
typedef struct OTDOA_HTTP_MEMBERS {
    bool bReady;
    bool bDownloadComplete;
    int nRange;      // Start offset for current request
    int nRangeSegmentEnd; // end of current segment
    int nRangeMax;   // HTTPS_RANGE_MAX_DEFAULT indicates unknown size
    int nHeaderLength;  // Length of header
    int nContentLength; // Length of content (body)
    int nBSARequests; // Number of BSA downloads this session
    int iHttpResponseCode;
    bool bDisableTLS;   // true to disable TLS and use HTTP (for uBSA DL only)
    bool bSkipConfigDL; // true to skip config file DL when DL the uBSA
    bool bSkipAuth;     // true to skip authentication when DL the uBSA
    bool bDisableEncryption;
    int nOverrideAuthResp;      // non-zero value overrides initial auth. response (for testing)
    uint8_t pubkey[PUBKEY_LMAX];
    uint8_t iv[IV_LMAX];

    char *pData;   // pointer to data
    char *csBuffer;
    char *szSend; // Send pointer usually points into csSend
    size_t nOff;
    int fdSocket;
    int fdFile;
    struct addrinfo *res;
    char szModemAddress[HELLAPHY_IP_ADDR_LMAX];
    char szServerAddress[HELLAPHY_IP_ADDR_LMAX];
//    char szURL_GET[HTTPS_URL_LMAX];
    char ubsa_token[UBSA_TOKEN_LMAX];
    unsigned uRecommendedDelay;
    unsigned uEcgi;
    unsigned uDlearfcn;
    unsigned uRadius;
    unsigned uNumCells;
    unsigned uNumRepeatCount;
    long prsID;

    tOTDOA_HTTP_BLACKLIST blacklist[BLACKLIST_SIZE];
} tOTDOA_HTTP_MEMBERS;

// Private members
// Connection
int http_bind(tOTDOA_HTTP_MEMBERS *, const char* pURL);
int http_connect(tOTDOA_HTTP_MEMBERS *, const char* tls_host);
int http_disconnect(tOTDOA_HTTP_MEMBERS* );
void http_unbind(tOTDOA_HTTP_MEMBERS *);
// uBSA Chunk Download
int http_format_request(tOTDOA_HTTP_MEMBERS *);
int http_send_request(tOTDOA_HTTP_MEMBERS *, size_t);
int http_show_response();

int http_write_chunk(tOTDOA_HTTP_MEMBERS *);
// Public members
void otdoa_http_init(void);
int otdoa_http_handle_message(tOTDOA_HTTP_MESSAGE *pMsg);
void otdoa_disable_tls(bool bDisableTLS);
void otdoa_skip_config_dl(bool bSkipConfigDL);
bool otdoa_get_skip_config_dl();
bool otdoa_get_disable_tls();
void otdoa_http_disable_encrypt(bool bDisableEncryption);
bool otdao_http_get_encryption_disable();
void otdoa_http_override_auth_resp(int override);
int otdoa_http_get_override_auth_resp();

int otdoa_http_send_ubsa_req(const char *const pURL, uint32_t u32ECGI, uint32_t u32DLEARFCN, uint32_t u32Radius,
                             uint32_t u32NumCells, bool reset_blacklist);
int otdoa_http_send_log_upload(const char *vf, uint32_t repeat);
int otdoa_http_send_vector_upload(const char *vf, uint32_t repeat);
int otdoa_http_send_results_upload(const char *pURL, otdoa_api_results_t *pResults,
                                   const char* p_notes, const char* p_true_lat, const char* p_true_lon);
int otdoa_http_send_test_jwt(void);
int otdoa_http_send_rebind_socket(void);
int otdoa_http_send_message(tOTDOA_HTTP_MESSAGE *pMsg);
int http_write_to_file(const char *path, void *data, size_t len) ;

bool otdoa_http_check_pending_stop();
void otdoa_http_invoke_callback_dl_compl(int status);
void otdoa_http_invoke_callback_ul_compl(int status);

const char *otdoa_http_get_download_url();
int cert_provision(const char* cert, size_t len);

#endif // ifndef OTDOA_HTTP_H
