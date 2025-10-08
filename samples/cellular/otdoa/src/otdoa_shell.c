
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
#include <otdoa_al/otdoa_shell.h>
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


void otdoa_shell_printf(void* psh, const char* pstring)
{
    shell_fprintf(psh, SHELL_NORMAL, "%s", pstring);
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

#if !CONFIG_OTDOA_SHELL_COMMANDS
/* The OTDOA command list needs to be created if it's not enabled by Kconfig */
SHELL_SUBCMD_SET_CREATE(otdoa_cmds, (phywi));
#endif

/* add custom commands to the otdoa command list */
SHELL_SUBCMD_ADD((phywi), show, &otdoa_cmds, " Display the uBSA file (nLines)", cmd_show_handler, 0, 2);
SHELL_SUBCMD_ADD((phywi), test, &otdoa_cmds, " Test OTDOA (Len,captFlags)", cmd_test_handler, 0, 2);
SHELL_SUBCMD_ADD((phywi), ecgi, &otdoa_cmds, " Override the serving cell ECGI - zero to reset, none to display current", cmd_override_handler, 0, 2);

#if !CONFIG_OTDOA_SHELL_COMMANDS
/* The OTDOA command list needs to be registered if it's not enabled by Kconfig */
SHELL_CMD_REGISTER(phywi, &otdoa_cmds, "PHY Wireless OTDOA Commands (non-NCS)", NULL);
#endif
