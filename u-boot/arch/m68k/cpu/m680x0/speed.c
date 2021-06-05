// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 *
 */

#include <common.h>
#include <clock_legacy.h>
#include <asm/global_data.h>
#include <asm/processor.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_M680x0
/* get_clocks() fills in gd->cpu_clock and gd->bus_clk */
int get_clocks(void)
{
	gd->bus_clk = CONFIG_SYS_CLK;
	gd->cpu_clk = CONFIG_SYS_CPU_CLK;

	return 0;
}
#endif
