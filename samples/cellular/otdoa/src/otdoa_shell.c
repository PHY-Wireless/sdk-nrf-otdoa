
/*------------------------------------------------------------------+
*     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
*       Copyright (C) Acorn Technologies, Inc.  2015-2021          |
*                   All Rights Reserved                            |
*------------------------------------------------------------------+
*
* otdoa_shell.c - User interface shell for OTDOA PoC
*
*/

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>
#include <otdoa_al/phywi_otdoa_api.h>
#include <otdoa_al/phywi_otdoa2al_api.h>
#include <version.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "nrf_modem_at.h"

#include "otdoa_sample_app.h"

// These APIs are not part of phywi_otdoa_api.h.  They are designed for
// testing only.  Use with caution!
int otdoa_nordic_at_get_ecgi_and_dlearfcn( uint32_t* pu32_ecgi, uint32_t* pu32_dlearfcn);
int otdoa_nordic_at_get_modem_version(char* psz_ver, unsigned max_len);
int otdoa_http_send_test_jwt(void);


void otdoa_shell_printf(void* psh, const char* pstring)
{
    shell_fprintf(psh, SHELL_NORMAL, "%s", pstring);
}

/*
 * cmd_get_ubsa_handler() - Handler for get_ubsa command.
 *
 *  This command has the following (optional) string parameters:
 *  1. Serving cell ECGI
 *  2. Radius
 *  3. NumCells
 */
static int cmd_get_ubsa_handler(const struct shell *shell, size_t argc,
                             char **argv)
{
    // Get ECGI & DLEARFCN
    uint32_t u32Ecgi = 0;               // A value of 0 causes OTDOA library to use the current serving cell ECGI
    uint32_t u32Dlearfcn = 5230;

    uint32_t u32Radius = 100000;        // Default to 100000 since v0.2 of server interprets this as meters
    uint32_t u32NumCells = 1000;        // increase to 1000 18 Jan 2022.  Patch for Atlanta GA field testing

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

    shell_print(shell, "Getting uBSA (ECGI: %u (0x%08x) DLEARFCN: %u  Radius: %u  Num Cells: %u)\n",
        u32Ecgi, u32Ecgi, u32Dlearfcn, u32Radius, u32NumCells);


    otdoa_sample_ubsa_dl_test(u32Ecgi, u32Dlearfcn, u32Radius, u32NumCells);
    return 0;
}

/*
 * cmd_get_config_handler() - Handler for get_config command
 *
 * Takes no parameters
 */
static int cmd_get_config_handler(const struct shell *shell, size_t argc, char **argv) {

    shell_print(shell, "Getting config file\n");

    int iRet = otdoa_api_cfg_download();

    if (iRet != 0) {
        shell_error(shell, "Failure in gettinig config file");
    }

    return 0;
}

/*
 * cmd_test_handler() - Handler for "test" command.
 *
 * This command has the following string parameters:
 *  1. Num PRS occasions
 *  2. Capture Flags
 */
static int cmd_test_handler(const struct shell *shell, size_t argc,
                             char **argv)
{
    uint32_t u32Length = 32;
    uint32_t u32CaptureFlags = 0;

    // NB: The first arg is the "test" subcommand
    // so argv[0] = "test", argv[1] = report interval, ...
    if (argc < 1)
        return -1;

    if (argc >= 2)
    {
        u32Length = strtoul(argv[1], NULL, 0);
    }
    if (argc >= 3) { u32CaptureFlags = strtoul(argv[2], NULL, 0); }

    uint32_t timeout_msec = OTDOA_SAMPLE_DEFAULT_TIMEOUT_MS;
    otdoa_sample_start(u32Length, u32CaptureFlags, timeout_msec);

    return 0;
}

/**
 * cmd_info_handler() - Handler for "info" command
 *
 * Displays the following:
 *  - ECGI of camped on cell
 *  - CTRL FSM state
 *  - NordicAT FSM state
 *  - Software version
 *  - ICCID
 */
static int cmd_info_handler(const struct shell *shell,
                            size_t argc,
                            char **argv)
{
    unsigned    ecgi;
    const char *version;
    char        iccid[256];
    int         rc;

    printk("Nordic OTDOA Sample\n");
    /* get the ECGI */
    rc = otdoa_nordic_at_get_ecgi_and_dlearfcn(&ecgi, NULL);
    if (rc)
        printk("Failed to get ECGI: %d\n", rc);
    else
        printk("          ECGI: %u\n", ecgi);


    /* get the software version */
    version = otdoa_api_get_version();
    printk("       Version: %s\n", version);

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
 * cmd_show_handler() - Handler for show command.
 *
 * This command has no parameters
 */
int ubsa_decode_file(const char* psz_infile, int i_display, const void* psh);


static int cmd_show_handler(const struct shell *shell, size_t argc,
                             char **argv)
{
    const char* psz_path = NULL;
    int i_display = 10000;  // max lines to display
    if (argc >= 2)
    {
        i_display = strtod(argv[1], NULL);
    }
    ubsa_decode_file(psz_path, i_display, shell);

    return 0;

}

/*
 * cmd_reset_handler() - Soft reset the device
 */
static int cmd_reset_handler(const struct shell *shell, size_t argc,
                             char **argv)
{

    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    NVIC_SystemReset();
    return 0;
}

//  Override the Serving Cell & DLEARFCN
extern uint32_t u32OverrideServCellECGI;
extern uint32_t u32OverrideDLEARFCN;
static int cmd_override_handler(const struct shell *shell, size_t argc, char **argv)
{
    uint32_t u32Ecgi = 0;
    uint32_t u32DlearFcn = 5230;
    otdoa_nordic_at_get_ecgi_and_dlearfcn(&u32Ecgi, &u32DlearFcn);
    printf("Current : INFO ECGI=%u, DLEARFCN=%u\n", u32Ecgi, u32DlearFcn);

    uint32_t u32ServCellECGI = 0;   // Zero means don't override
    if (argc >= 2)
    {
        u32ServCellECGI = strtoul(argv[1], NULL, 0);    // Zero means don't override
        u32OverrideServCellECGI = u32ServCellECGI;
        if (u32ServCellECGI)
        {
            u32OverrideDLEARFCN = u32DlearFcn;
        }
    }
    if (argc > 2)
    {
        u32OverrideDLEARFCN = strtoul(argv[2], NULL, 0);
    }

    printf("Serving Cell Override ECGI=%u DLEARFCN=%u\n", u32OverrideServCellECGI, u32OverrideDLEARFCN);
    return 0;
}

static void cmd_provision_handler(const struct shell *shell, size_t argc, char **argv) 
{
    otdoa_api_provision(argv[1]);
}

static void cmd_jwt_handler(const struct shell *shell, size_t argc, char **argv) {
    otdoa_http_send_test_jwt();
}


/* Creating subcommands (level 1 command) array for command "phywi". */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_phywi,
        SHELL_CMD_ARG( get_ubsa,   NULL, " Download a uBSA (ECGI,DLEARFCN,Radius,NumCells)", cmd_get_ubsa_handler, 0, 4),

        SHELL_CMD(     reset,      NULL, " Soft reset the device", cmd_reset_handler),
        SHELL_CMD_ARG( show,       NULL, " Display the uBSA file (nLines)", cmd_show_handler, 0, 2),
        SHELL_CMD_ARG( test,       NULL, " Test OTDOA (Len,captFlags)", cmd_test_handler, 0, 2),
        SHELL_CMD(     info,       NULL, " Show current OTDOA info", cmd_info_handler),

        SHELL_CMD_ARG(ecgi,        NULL, " Override the serving cell ECGI - zero to reset, none to display current", cmd_override_handler, 0, 2),

        SHELL_CMD( get_config,     NULL, " Download a config file", cmd_get_config_handler),
        SHELL_CMD(jwt,             NULL, " Test generate JWT token", cmd_jwt_handler),

        SHELL_CMD_ARG(provision,   NULL, " Provision a key to use for JWT generation", cmd_provision_handler, 0, 1),
        SHELL_SUBCMD_SET_END
        );
/* Creating root (level 0) command "phywi" */
SHELL_CMD_REGISTER(phywi, &sub_phywi, "PHY Wireless OTDOA Commands", NULL);

