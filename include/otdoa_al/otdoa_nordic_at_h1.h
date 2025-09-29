/*------------------------------------------------------------------+
 *     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
 *       Copyright (C) Acorn Technologies, Inc.  2015-2024          |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdao_nordic_at_h1.c - AT command/response 
 *                        for Nordic-Based "H1" product
 *
 */
#ifndef INCLUDE_OTDOA_NORDIC_AT_H1_H_
#define INCLUDE_OTDOA_NORDIC_AT_H1_H_

#include <stdint.h>
#include <stddef.h>

// @brief Parse the response to AT%%XMONITOR and return ECGI & DLEARFCN
/// @param psz_resp[in]         response string returned by the modem
/// @param u_resp_len[in]       Length of the response string 
/// @param pu32_ecgi[out]       ECGI   
/// @param pu32_dlearfcn[out]   DLEARFCN
/// @return     0               Success
///
/// @note response is documented in "nRF91 AT Commands Command Reference Guide" v2.0
/// it has the form: 
///             0 1                  2     3        4      5 6  7          8   9    10 11 12 13         14         15
///  %XMONITOR: 1,"Verizon Wireless","VZW","311480","2F02",7,13,"00B7F901",282,5230,37,25,"","11100000","11100000","01011110"
//
int otdoa_nordic_at_parse_xmonitor_response(
        const char* const psz_resp, size_t u_resp_len,
        uint32_t* pu32_ecgi, uint32_t* pu32_dlearfcn);


/// @brief Use AT%%XMONITOR command to get the current ECGI and DLEARFCN from the modem
/// @param pu32_ecgi[out]       A pointer to where the returned ECGI will be written
/// @param pu32_dlearfcn[out]   A pointer to where the returned DLEARFCN will be written
/// @return  0 on success
///          values from otdoa_api_error_codes_t (in phywi_otdoa_api.h) on any failure
int otdoa_nordic_at_get_ecgi_and_dlearfcn( uint32_t *pu32_ecgi, uint32_t *pu32_dlearfcn);



/// @brief Gets the IMEI from the modem
/// @return 0 on success
///         OTDOA_EVENT_FAIL_BAD_MODEM_RESP for any failure
int otdoa_nordic_at_get_imei_from_modem();


/// @brief returns the current IMEI value
/// @return pointer to the IMEI value
const char* otdoa_get_imei_string();

/// @brief Our own version of str_tok_r that stops on the first token found.
/// So it doesn't eat consecutive delimeters
char* otdoa_nordic_at_strtok_r (char *s, char delim, char **save_ptr);

/// @brief Returns the version string from the modem firmware
/// @param psz_ver points to where the version string will be written
/// @param max_len Maximum length of data to write
/// @return 0 on success
int otdoa_nordic_at_get_modem_version(char* psz_ver, unsigned max_len);

#endif /* INCLUDE_OTDOA_NORDIC_AT_H1_H_ */
