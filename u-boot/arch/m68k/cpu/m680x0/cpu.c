// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 *
 */

#include <common.h>
#include <command.h>
#include <init.h>
#include <vsprintf.h>
#include <asm/immap.h>
#include <asm/io.h>

#ifdef CONFIG_M680x0
__weak int m680x0_do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printf("Need to implement platform specific reset function.\n");
	return -1;
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return m680x0_do_reset(cmdtp, flag, argc, argv);
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

#ifdef CONFIG_M68030
	printf("CPU:   Motorola M68030 at %s MHz\n", strmhz(buf, CONFIG_SYS_CPU_CLK));
#endif
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */
#endif
