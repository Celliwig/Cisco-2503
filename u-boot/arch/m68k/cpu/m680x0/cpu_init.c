// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 *
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <watchdog.h>
#include <asm/immap.h>
#include <asm/io.h>

#if defined(CONFIG_M680x0)
/*
 * CPU specific initialisation
 */
void cpu_init_f(void)
{
#if defined(CONFIG_M68030)
	/* enable data/instruction cache now */
	icache_enable();
	dcache_enable();
#endif
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
	return 0;
}

//void arch_preboot_os(void)
//{
//	/*
//	 * OS can change interrupt offsets and are about to boot the OS so
//	 * we need to make sure we disable all async interrupts.
//	 */
//	intctrl_t *icr = (intctrl_t *)(MMAP_INTC);
//
//	out_8(&icr->icr1, 0x00); /* timer 1     */
//	out_8(&icr->icr2, 0x00); /* timer 2     */
//}
#endif
