/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#ifndef NORDIC_OTDOA_APP_OTDOA_SHELL_H
#define NORDIC_OTDOA_APP_OTDOA_SHELL_H

#if CONFIG_OTDOA_SHELL_COMMANDS

#include <zephyr/shell/shell.h>

SHELL_SUBCMD_SET_CREATE(otdoa_cmds, (phywi));

#endif

#endif //NORDIC_OTDOA_APP_OTDOA_SHELL_H