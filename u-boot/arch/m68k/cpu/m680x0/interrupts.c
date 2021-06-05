// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 *
 */

#include <common.h>
#include <irq_func.h>
#include <asm/immap.h>
#include <asm/io.h>

#ifdef CONFIG_M680x0
int interrupt_init(void)
{
	enable_interrupts();

	return 0;
}

#endif
