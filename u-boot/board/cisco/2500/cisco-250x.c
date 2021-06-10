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

/*
 * Low level board init
 * A7 contains return address as stack not available
 */
void lowlevel_init(void) {
	/* Don't know if this is necessary */
	/* Was in the original firmware */
	__asm__ __volatile__ ("		move.l	#0x3e8, %d0");
	__asm__ __volatile__ ("1:	subq.l	#0x1, %d0");
	__asm__ __volatile__ ("		bgt.s	1b");

        /* Swap out boot ROM for RAM */
	__asm__ __volatile__ ("		movea.l	#0x2110000, %a0");
	__asm__ __volatile__ ("		move.w	(%a0), %d0");
	__asm__ __volatile__ ("		andi.w	#0xfffe, %d0");
	__asm__ __volatile__ ("		move.w	%d0, (%a0)");
	__asm__ __volatile__ ("		jmp	(%a7)");
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

#ifdef CONFIG_BOARD_EARLY_INIT_R
// U-boot relocated so enable status LED
int board_early_init_r(void) {
	// Enable status LED
	writew((readw(CISCO2500_REG_SYSCTRL0) | CISCO2500_REG_SYSCTRL1_STATUS_LED), CISCO2500_REG_SYSCTRL0);

	return 0;
}
#endif

// Implement reset function by using watchdog
int m680x0_do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printf("Resetting Cisco...\n");
	writew(0x3e80, CISCO2500_REG_WDOG_RST);			// Counter value?
	writeb(0x40, CISCO2500_REG_TIMER_CTL);			// Start watchdog
	while (true) {						// Loop forever
		__asm__ __volatile__ ("nop");
	}
	return -1;
}

/*
 * Architectural timer functions
 */
void m680x0_timer_cfg(void)
{
	// Set timer value
	// (1MHz clock, 1000 -> 1ms -> 1000Hz)
	writew(0x03E8, CISCO2500_REG_TIMER_VAL);

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

/*
 * Define flash to deal with interleaved
 * boot flash with reversed bus mapping
 */
#if defined(CONFIG_FLASH_CFI_LEGACY)
#include <flash.h>
#include <mtd/cfi_flash.h>
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t * info)
{
	if (banknum == 0) {
//		info->flash_id          = 0x01000000;
		info->portwidth         = FLASH_CFI_16BIT;
		info->chipwidth         = FLASH_CFI_BY8;
//		info->buffer_size       = 1;
		info->erase_blk_tout    = 16384;
		info->write_tout        = 2;
//		info->buffer_write_tout = 5;
		info->vendor            = CFI_CMDSET_AMD_LEGACY;
//		info->cmd_reset         = 0x00F0;
		info->interface         = FLASH_CFI_X16;
		//info->chip_lsb	= 1;
//		info->legacy_unlock     = 0;
		info->manufacturer_id   = (u16) AMD_MANUFACT;
//		info->device_id         = ATM_ID_LV040;
//		info->device_id2        = 0;

//		info->ext_addr          = 0;
//		info->cfi_version       = 0x3133;
//		info->cfi_offset        = 0x0000;
		info->addr_unlock1      = 0x00000555;
		info->addr_unlock2      = 0x000002AA;
		info->name              = "Ganged Am29F040B";

		info->base		= 0x01000000;
		info->size              = 0x100000;
		info->sector_count      = 0x08;
		for (int i = 0; i < info->sector_count; i++) {
			info->start[i] = info->base + (i * (info->size / info->sector_count));
			info->protect[i] = 0;
		}

		return 1;
	}

	return 0;
}

#endif				/* CONFIG_SYS_FLASH_CFI */
