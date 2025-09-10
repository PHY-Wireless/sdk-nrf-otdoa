/*------------------------------------------------------------------+
 *     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
 *       Copyright (C) Acorn Technologies, Inc.  2015-2022          |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdoa_log.h
 *
 */
#ifndef _OTDOA_LOG_H_
#define _OTDOA_LOG_H_

#include <stdarg.h>

#define OTDOA_MAX_LOG_LENGTH  96

unsigned otdoa_log_level_set(int level, const char *backend);
void otdoa_log_err(const char *fmt, ...);
void otdoa_log_wrn(const char *fmt, ...);
void otdoa_log_inf(const char *fmt, ...);
void otdoa_log_dbg(const char *fmt, ...);
void otdoa_log_hexdump_inf(void* data, unsigned length, char* str);

void otdoa_log_init();

#endif /* _OTDOA_LOG_H_ */
