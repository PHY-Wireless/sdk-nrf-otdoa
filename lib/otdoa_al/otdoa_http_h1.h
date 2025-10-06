/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#ifndef OTDOA_HTTP_H1_H
#define OTDOA_HTTP_H1_H

#include <otdoa_al/phywi_otdoa_api.h>

/* ---------------------------------------------------------
                           TYPES
   --------------------------------------------------------- */
int  otdoa_http_h1_format_auth_request(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_format_range_request(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_handle_message(tOTDOA_HTTP_MESSAGE*);
void otdoa_http_h1_init_config_dl();
int  otdoa_http_h1_download_ubsa(tOTDOA_HTTP_MEMBERS* p_http, tOTDOA_HTTP_MESSAGE *params);
int  otdoa_http_h1_send_get_ubsa_auth_req(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_send_get_ubsa_range_req(tOTDOA_HTTP_MEMBERS*);
int  otdoa_http_h1_parse_response_code(const char*);
otdoa_api_error_codes_t otdoa_http_h1_process_config_response_header(tOTDOA_HTTP_MEMBERS*);
otdoa_api_error_codes_t otdoa_http_h1_process_auth_response(tOTDOA_HTTP_MEMBERS*);
otdoa_api_error_codes_t otdoa_http_h1_process_range_response_header(tOTDOA_HTTP_MEMBERS*);
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
