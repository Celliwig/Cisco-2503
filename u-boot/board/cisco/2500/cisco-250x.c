// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Cisco 250x Router
 *
 * (C) Copyright 2021 Celliwig
 *
 * This file copies memory testdram() from sandburst/common/sb_common.c
 */

#include <common.h>
//#include <init.h>
#include <asm/global_data.h>
//#include <asm/immap.h>
//#include <asm/io.h>
//#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

void lowlevel_init(void) {
        /* Swap out boot ROM for RAM */
	__asm__ __volatile__ ("movea.l	#0x2110000, %a0");
	__asm__ __volatile__ ("move.w	(%a0), %d0");
	__asm__ __volatile__ ("andi.w	#0xfffe, %d0");
	__asm__ __volatile__ ("move.w	%d0, (%a0)");
	__asm__ __volatile__ ("jmp	(%a7)");
}

int checkboard(void)
{
	puts("Board: ");
	puts("Cisco 2500\n");

	return 0;
}

int dram_init(void)
{
	// Enable RAM parity here?
	// Detect RAM size
	// Set RAM window

	// Set RAM size
        gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
