/*------------------------------------------------------------------+
 *        ACORN TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL           |
 *       Copyright (C)  PHY Wireless, LLC a wholly owned subsidiary |
 *             of Acorn Technologies, Inc.  2015-2025               |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdoa_al_api.h - API Definitions for Adaption Layer
 *
 */

#ifndef PHYWI_AL_API__
#define PHYWI_AL_API__

#include <stdint.h>
#include <stddef.h>
#include "phywi_otdoa_api.h"

#ifdef __cplusplus
extern "C" {
#endif


 /**
 * @brief Initialize the OTDOA AL library
 * @param[in] ubsa_file_path Points to a string containing the full path to where
 *                           the uBSA file resides
 * @param[in] callback Callback function used by the library
 *                     to return results and status to the client
 * @return 0 on success
 */
int32_t otdoa_al_init(otdoa_api_callback_t event_callback);



#ifdef __cplusplus
}
#endif
#endif  // PHYWI_AL_API__
