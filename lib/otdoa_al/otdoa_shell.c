/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#if CONFIG_OTDOA_SHELL_COMMANDS

#include <stddef.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>

#include "otdoa_al/otdoa_shell.h"
#include "otdoa_al/phywi_otdoa_api.h"
#include "otdoa_al/otdoa_nordic_at_h1.h"
#include "nrf_modem_at.h"

/**
 *
 * otdoa_shell_info_handler() - Handler for "info" command
 *
 * Displays the following:
 *  - ECGI of camped on cell
 *  - CTRL FSM state
 *  - NordicAT FSM state
 *  - Software version
 *  - ICCID
 */
static int otdoa_shell_info_handler(const struct shell *shell,
                            size_t argc,
                            char **argv)
{
    unsigned    ecgi;
    const char *version;
    char        iccid[256];
    int         rc;

    printk("Nordic OTDOA Application\n");
    /* get the ECGI */
    rc = otdoa_nordic_at_get_ecgi_and_dlearfcn(&ecgi, NULL);
    if (rc)
        printk("Failed to get ECGI: %d\n", rc);
    else
        printk("          ECGI: %u\n", ecgi);

    /* get the software version */
    version = otdoa_api_get_version();
    printk("       Version: %s\n", version);
#ifdef APP_MODEM_TRACE_ENABLED
    printk("         Trace: ENABLED\n");
#else
    printk("         Trace: DISABLED\n");
#endif

    rc = otdoa_nordic_at_get_modem_version(iccid, sizeof(iccid));
    if (rc) printk("Failed to get Modem Version\n");
    else {
        printk("   MFW Version: %s\n", iccid);
    }

    const char* pszIMEI = otdoa_get_imei_string();
    if (NULL == pszIMEI)
        pszIMEI = "unknown";
    printk("          IMEI: %s\n", pszIMEI);

    /* get the ICCID */
    rc = nrf_modem_at_cmd(iccid, sizeof iccid, "AT%%XICCID");
    if (rc)
        printk("Failed to get ICCID: %d\n", rc);
    else {
        /* find the end of the ICCID and end the string there */
        *strchr(iccid, '\n') = 0;
        printk("       %s\n", iccid);
    }

    return 0;
}

/*
 * otdoa_shell_get_config_handler() - Handler for get_config command
 *
 * Takes no parameters
 */
static int otdoa_shell_get_config_handler(const struct shell *shell, size_t argc, char **argv) {

    shell_print(shell, "Getting config file\n");

    int iRet = otdoa_api_cfg_download();

    if (iRet != 0) {
        shell_error(shell, "Failure in gettinig config file");
    }

    return 0;
}

extern int otdoa_http_send_test_jwt(void);
static void otdoa_shell_jwt_handler(const struct shell *shell, size_t argc, char **argv) {
    otdoa_http_send_test_jwt();
}

/*
 * cmd_get_ubsa_handler() - Handler for get_ubsa command.
 *
 *  This command has the following (optional) string parameters:
 *  1. Serving cell ECGI
 *  2. Radius
 *  3. NumCells
 */
extern uint32_t u32OverrideDLEARFCN;
static int otdoa_shell_get_ubsa_handler(const struct shell *shell, size_t argc,
                             char **argv)
{
    // Get ECGI & DLEARFCN
    uint32_t u32Ecgi = 0;           // Value of ECGI causes OTDOA library to use current serving cell
    uint32_t u32Dlearfcn = 5230;

    uint32_t u32Radius = 100000;        // Default to 100000 since v0.2 of server interprets this as meters
    uint32_t u32NumCells = 1000;        // increase to 1000 18 Jan 2022.  Patch for Atlanta GA field testing
    uint32_t u32NumRepeatCount = 0;     // Special-case - behave as before

    if (u32OverrideDLEARFCN > 0) u32Dlearfcn = u32OverrideDLEARFCN;

    if (argc >= 2)
    {
        u32Ecgi = strtoul(argv[1], NULL, 0);
    }

    if (argc >= 3)
    {
        u32Radius = strtoul(argv[2], NULL, 0);
        if (0 == u32Radius)
        {
            shell_error(shell, "Failed to convert radius (%s)\n", argv[2]);
        }
    }

    if (argc >= 4)
    {
        u32NumCells = strtoul(argv[3], NULL, 0);
        if (0 == u32NumCells)
        {
            shell_error(shell, "Failed to convert number of cells (%s)\n", argv[3]);
        }
    }

    shell_print(shell, "Getting uBSA (ECGI: %u (0x%08x) DLEARFCN: %u  Radius: %u  Num Cells: %u, u32NumRepeatCount=%u)\n",
        u32Ecgi, u32Ecgi, u32Dlearfcn, u32Radius, u32NumCells, u32NumRepeatCount);

    //otdoa_sample_ubsa_dl_test(u32Ecgi, u32Dlearfcn, u32Radius, u32NumCells);
    otdoa_api_ubsa_dl_req_t dl_req = { 0 };
    dl_req.ecgi = u32Ecgi;
    dl_req.dlearfcn = u32Dlearfcn;
    dl_req.ubsa_radius_meters = u32Radius;
    dl_req.max_cells = u32NumCells;
    int err = otdoa_api_ubsa_download(&dl_req, CONFIG_OTDOA_DEFAULT_UBSA_PATH, true);
    if (err != OTDOA_API_SUCCESS) {
        shell_error(shell, "uBSA download failed with return %d\n", err);
    }
    return 0;
}

static void otdoa_shell_provision_handler(const struct shell *shell, size_t argc, char **argv) {
    otdoa_api_provision(argv[1]);
}

/*
 * cmd_reset_handler() - Soft reset the device
 */
static int otdoa_shell_reset_handler(const struct shell *shell, size_t argc,
                             char **argv)
{

    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    NVIC_SystemReset();
    return 0;
}

SHELL_SUBCMD_ADD((phywi), info,       &otdoa_cmds, "Show current OTDOA info", otdoa_shell_info_handler, 0, 0);
SHELL_SUBCMD_ADD((phywi), get_config, &otdoa_cmds, "Download a config file",  otdoa_shell_get_config_handler, 0, 0);
SHELL_SUBCMD_ADD((phywi), jwt,        &otdoa_cmds, "Test generate JWT token", otdoa_shell_jwt_handler, 0, 0);
SHELL_SUBCMD_ADD((phywi), get_ubsa,   &otdoa_cmds, "Download a uBSA (ECGI, Radius, NumCells)", otdoa_shell_get_ubsa_handler, 0, 4);
SHELL_SUBCMD_ADD((phywi), provision,  &otdoa_cmds, "Provision a key to use for JWT generation", otdoa_shell_provision_handler, 0, 1);
SHELL_SUBCMD_ADD((phywi), reset,      &otdoa_cmds, "Soft reset the device", otdoa_shell_reset_handler, 0, 0);

SHELL_CMD_REGISTER(phywi, &otdoa_cmds, "PHY Wireless OTDOA Commands", NULL);

#endif