// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Cisco 250x Router
 *
 * (C) Copyright 2021 Celliwig
 *
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
//#include <asm/immap.h>
#include <asm/io.h>
#include <asm/m680x0_time.h>
//#include <dm.h>
#include "cisco-250x.h"

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
	// Board detection routine here?

	puts("Board: ");
	puts("Cisco 2503\n");

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

/*
 * Architectural timer functions
 */
void m680x0_timer_cfg(void)
{
	// Set timer value
	// (4000 - 40MHz clock)
	writew(0x0FA0, CISCO2500_REG_TIMER_VAL);

	// Don't need to configure interrupts,
	// it's attached to NMI
}

void m680x0_timer_irq_handler(void *not_used)
{
	// Clear timer status
	readb(CISCO2500_REG_TIMER_CTL);

	// Increment time
	m680x0_timestamp += 1;
}

void m680x0_timer_start(void)
{
	// Start timer
	writeb(0x00, CISCO2500_REG_TIMER_CTL);
	// Clear timer status
	readb(CISCO2500_REG_TIMER_CTL);
}

void m680x0_timer_stop(void)
{
	// Disable timer
	writew(0x0001, CISCO2500_REG_TIMER_VAL);
	// Clear timer status
	readb(CISCO2500_REG_TIMER_CTL);
}

void m680x0_timer_udelay(unsigned long usec_delay)
{
	u32 cpu_cycle_delay;
	unsigned long delay_time;

	// If the delay is greater than the period of the system timer use that
	if (usec_delay > ARCH_TIMER_PERIOD_IN_USEC) {
		// Calculate delay
		delay_time = get_ticks() + usec2ticks(usec_delay);

		// Wait for time to expire
		while (get_ticks() < delay_time) {}
	} else {
		// Calculate delay using DBcc which takes 6 cycles (68030)
		cpu_cycle_delay = (usec_delay * M680X0_CPU_CYCLES_PER_USEC) / 6;
		__asm__ __volatile__("1:	dbf %0, 1b"::"r"(cpu_cycle_delay));
	}
}