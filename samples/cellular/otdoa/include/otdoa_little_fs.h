/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#ifndef SRC_INCLUDE_OTDOA_FILE_H_
#define SRC_INCLUDE_OTDOA_FILE_H_

#ifndef OTDOA_LITTLE_FS
#error "OTDOA_LITTLE_FS not defined"
#endif

#if OTDOA_LITTLE_FS == 0
int mount_fat_fs(void);
int test_fs_raw(void);
int test_fs(void);

#else
int mount_little_fs(void);
int unmount_little_fs(void);
int test_little_fs(void);

#endif

#endif /* SRC_INCLUDE_OTDOA_FILE_H_ */
