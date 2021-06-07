// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 *
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <init.h>
#include <irq_func.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm/m680x0.h>
#include <asm/m680x0_time.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_M680x0)

extern	void	m680x0_timer_cfg(void);
extern	void	m680x0_timer_irq_handler(void *not_used);
extern	void	m680x0_timer_start(void);
extern	void	m680x0_timer_stop(void);
extern	void	m680x0_timer_udelay(unsigned long usec);

#if !defined(CONFIG_SYS_TMRINTR_NO)
#	error	"TMRINTR_NO not defined!"
#endif

void __udelay(unsigned long usec)
{
	m680x0_timer_udelay(usec);
}

int timer_init(void)
{
	m680x0_timestamp = 0;

	// Setup architectural timer
	m680x0_timer_cfg();

	/* initialize and enable timer interrupt */
	irq_install_handler(CONFIG_SYS_TMRINTR_NO, m680x0_timer_irq_handler, 0);

	m680x0_timer_start();

	return 0;
}

ulong get_timer(ulong base)
{
	return (m680x0_timestamp - base);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On M68K it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return m680x0_timestamp;
}

unsigned long usec2ticks(unsigned long usec)
{
	unsigned int ticks = (usec / ARCH_TIMER_PERIOD_IN_USEC);
	// Round up if necessary
	if ((usec % ARCH_TIMER_PERIOD_IN_USEC) >= (ARCH_TIMER_PERIOD_IN_USEC / 2))
		ticks++;
	return ticks;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On M68K it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

#endif	/* CONFIG_M680x0 */
