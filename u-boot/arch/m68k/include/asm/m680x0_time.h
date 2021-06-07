/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Celliwig
 *
 */

#define	ARCH_TIMER_PERIOD_IN_USEC	(1000000 / CONFIG_SYS_HZ)
#define	M680X0_CPU_CYCLES_PER_USEC	(CONFIG_SYS_CPU_CLK / 1000000)

unsigned long m680x0_timestamp;
