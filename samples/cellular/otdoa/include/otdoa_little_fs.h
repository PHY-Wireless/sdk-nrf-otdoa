/*------------------------------------------------------------------+
 *     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
 *       Copyright (C) Acorn Technologies, Inc.  2015-2020          |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdoa_file.h
 *
 */
#ifndef SRC_INCLUDE_OTDOA_FILE_H_
#define SRC_INCLUDE_OTDOA_FILE_H_

#ifndef OTDOA_LITTLE_FS
#error "OTDOA_LITTLE_FS not defined"
#endif

#if OTDOA_LITTLE_FS == 0
int mount_fat_fs();
int test_fs_raw();
int test_fs();

#else
int mount_little_fs();
int unmount_little_fs();
int test_little_fs();


#endif


#endif /* SRC_INCLUDE_OTDOA_FILE_H_ */
