/*------------------------------------------------------------------+
 *     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
 *       Copyright (C) Acorn Technologies, Inc.  2015-2023          |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdoa_http_h1.h
 *
 * OTDOA HTTP Module for "H1" product
 *
 */

#ifndef OTDOA_HTTP_H1_H
#define OTDOA_HTTP_H1_H

/* ---------------------------------------------------------
                           TYPES
   --------------------------------------------------------- */
typedef enum {
     HTTP_H1_ERROR = -1,
     HTTP_H1_CANCELLED = -2,
     HTTP_H1_MESSAGE_ERROR = -3,
     HTTP_H1_REGISTRATION_ERROR = -4,
     HTTP_H1_OK = 0,                // 200
     HTTP_H1_NOT_READY,             // 202
     HTTP_H1_PARTIAL_CONTENT,       // 206
     HTTP_H1_BAD_REQUEST,           // 400
     HTTP_H1_UNAUTHORIZED,          // 401
     HTTP_H1_CONFLICT,              // 409
     HTTP_H1_GONE,                  // 410
     HTTP_H1_UNPROCESSABLE_CONTENT, // 422
     HTTP_H1_TOO_MANY_REQUESTS,     // 429
     HTTP_H1_INTERNAL_SERVER_ERROR, //500

     HTTP_H1_BAD_CFG,               // Failure in config DL
} tOTDOA_HTTP_H1_RESPONSE_STATUS;

int  otdoa_http_h1_format_auth_request(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_format_range_request(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_handle_message(tOTDOA_HTTP_MESSAGE*);
void otdoa_http_h1_init_config_dl();
int  otdoa_http_h1_download_ubsa(tOTDOA_HTTP_MEMBERS* p_http, tOTDOA_HTTP_MESSAGE *params);
int  otdoa_http_h1_send_get_ubsa_auth_req(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_send_get_ubsa_range_req(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_parse_response_code(const char*);
tOTDOA_HTTP_H1_RESPONSE_STATUS  otdoa_http_h1_process_config_response_header(tOTDOA_HTTP_MEMBERS*);
tOTDOA_HTTP_H1_RESPONSE_STATUS  otdoa_http_h1_process_auth_response(tOTDOA_HTTP_MEMBERS*);
tOTDOA_HTTP_H1_RESPONSE_STATUS  otdoa_http_h1_process_range_response_header(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_process_config_response_content(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_process_range_response_content(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_process_ubsa_data(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_receive_header(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_receive_content(tOTDOA_HTTP_MEMBERS*, int iContentLen);
int  otdoa_http_h1_get_header_len(const char* pBuffer);
char* otdoa_http_h1_find_value(char*, const char*, int);
int  otdoa_http_h1_split_into_tokens(char*);
int  otdoa_http_h1_rebind(const char* url);
int  otdoa_http_h1_format_cfg_request(tOTDOA_HTTP_MEMBERS *pG);
int  otdoa_http_h1_send_get_cfg_req(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_handle_get_cfg(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_handle_otdoa_results(tOTDOA_HTTP_MEMBERS*, const tOTDOA_HTTP_MESSAGE*);
int  otdoa_http_h1_blacklist_check(tOTDOA_HTTP_MEMBERS*, unsigned ecgi);
int  otdoa_http_h1_blacklist_add(tOTDOA_HTTP_MEMBERS*, unsigned ecgi);
int  otdoa_http_h1_blacklist_clear(tOTDOA_HTTP_MEMBERS*, unsigned ecgi);
int  otdoa_http_h1_blacklist_init(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_blacklist_tick(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_test_jwt(tOTDOA_HTTP_MEMBERS*);

// These are made public for support of unit tests
int  otdoa_http_h1_free_cs_buffer();
int  otdoa_http_h1_get_cs_buffer();


#endif // ifndef OTDOA_HTTP_H1_H
