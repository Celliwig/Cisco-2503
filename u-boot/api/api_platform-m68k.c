// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 *
 * This file contains routines that fetch data from PowerPC-dependent sources
 * (bd_info etc.)
 */

#include <config.h>
#include <linux/types.h>
#include <api_public.h>

#include <asm/u-boot.h>
#include <asm/global_data.h>

#include "api_private.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Important notice: handling of individual fields MUST be kept in sync with
 * include/asm-ppc/u-boot.h and include/asm-ppc/global_data.h, so any changes
 * need to reflect their current state and layout of structures involved!
 */
int platform_sys_info(struct sys_info *si)
{
	si->clk_bus = gd->bus_clk;
	si->clk_cpu = gd->cpu_clk;

	platform_set_mr(si, gd->ram_base, gd->ram_size, MR_ATTR_DRAM);
	platform_set_mr(si, gd->bd->bi_flashstart, gd->bd->bi_flashsize, MR_ATTR_FLASH);

	return 1;
}