// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <exports.h>
#include <asm/io.h>
#include <asm/m680x0.h>
#include "../../board/cisco/2500/cisco-250x.h"
#include "../../drivers/net/lance_am79C90.h"

#define	MAXIMUM_ETHERNET_FRAME_SIZE	1524

bool check_addr(void *ptr, unsigned int check_bits);
void lance_irq_handler(void *not_used);

static unsigned char lance_initialised;

int lance_test(int argc, char *const argv[])
{
	struct lance_init_block __attribute__ ((aligned (1))) init_block;
	struct lance_rx_msg_descript __attribute__ ((aligned (8))) rx_msg;
	struct lance_tx_msg_descript __attribute__ ((aligned (8))) tx_msg;
	unsigned char rx_buffer[MAXIMUM_ETHERNET_FRAME_SIZE];
	unsigned char tx_buffer[MAXIMUM_ETHERNET_FRAME_SIZE];
	ulong now;

	/* Setup App */
	app_startup(argv);

	printf ("LANCE ethernet controller test\n");
	lance_initialised = 0;

	//// Setup interrupt handler
	//install_hdlr(EVA_INTERRUPT_LEVEL4, &lance_irq_handler, 0);

	// Enable LANCE
	writeb(readb(CISCO2500_REG_SYSCTRL5) & ~CISCO2500_REG_SYSCTRL5_RSTCTRL_LANCE, CISCO2500_REG_SYSCTRL5);

	//// Enable LANCE IRQ
	//writeb(readb(CISCO2500_REG_SYSCTRL7) & ~CISCO2500_REG_SYSCTRL7_IRQEN_LANCE, CISCO2500_REG_SYSCTRL7);

	// Ensure LANCE stopped
	writew(0x0, CISCO2500_LANCE_ADDR_RAP);								/* Select CSR0 */
	writew(LANCE_AM79C90_CSR0_CTRL_STOP, CISCO2500_LANCE_ADDR_RDP);					/* Set STOP */

	// Check initialisation block alignment
	if (!check_addr(&init_block, 0x1)) {
		printf("Initialisation Block must be word aligned and in the lower 24bits of memory!!!\n");
		goto error;
	}

	// Rx message descriptor alignment
	if (!check_addr(&rx_msg, 0x7)) {
		printf("Rx message descriptor must be quad word aligned and in the lower 24bits of memory!!!\n");
		goto error;
	}

	// Tx message descriptor alignment
	if (!check_addr(&tx_msg, 0x7)) {
		printf("Tx message descriptor must be quad word aligned and in the lower 24bits of memory!!!\n");
		goto error;
	}

	// LE
	// Configure initialisation block
	init_block.mode			= 0x0003;
	init_block.phy_addr1		= 0xe000;							/* Ethernet Address */
	init_block.phy_addr2		= 0xb91e;
	init_block.phy_addr3		= 0x9123;
	init_block.laddr_filter1	= 0x0000;							/* Disable Logical Address Filter */
	init_block.laddr_filter2	= 0x0000;							/* Disables Multicast */
	init_block.laddr_filter3	= 0x0000;
	init_block.laddr_filter4	= 0x0000;
	init_block.rdr_ptr1		= (u32) &rx_msg & 0xffff;
	init_block.rdr_ptr2		= (((u32) &rx_msg & 0xff0000) >> 16) | LANCE_AM79C90_DRSIZE_1;
	init_block.tdr_ptr1		= (u32) &tx_msg & 0xffff;
	init_block.tdr_ptr2		= (((u32) &tx_msg & 0xff0000) >> 16) | LANCE_AM79C90_DRSIZE_1;

	// Configure Rx Message Descriptor
	rx_msg.rmd0 = (u32) &rx_buffer & 0xffff;							/* Lower 16 bit of Rx buffer address */
	rx_msg.rmd1 = (((u32) &rx_buffer & 0xff0000) >> 16) | LANCE_AM79C90_RMD1_OWN;			/* Upper 8 bits of Rx buffer address, mark as belonging to LANCE */
	rx_msg.rmd2 = 0xf000 | ~(MAXIMUM_ETHERNET_FRAME_SIZE - 1);					/* Upper 8 bits must be set, negative 2's complement buffer size */
	rx_msg.rmd3 = 0x0000;										/* Clear message byte count */

	// Configure Tx Message Descriptor
	tx_msg.tmd0 = (u32) &tx_buffer & 0xffff;							/* Lower 16 bit of Tx buffer address */
	tx_msg.tmd1 = ((u32) &tx_buffer & 0xff0000) >> 16;						/* Upper 8 bits of Tx buffer address, mark as belonging to Host */
	tx_msg.tmd2 = 0xf000 | ~(MAXIMUM_ETHERNET_FRAME_SIZE - 1);					/* Upper 8 bits must be set, negative 2's complement buffer size */
	tx_msg.tmd3 = 0x0000;										/* Clear errors */

	// Set LANCE initialisation address
	writew(0x1, CISCO2500_LANCE_ADDR_RAP);								/* Select CSR1 */
	writew((u32) &init_block & 0xffff, CISCO2500_LANCE_ADDR_RDP);					/* Initialisation Block Lower Address */
	writew(0x2, CISCO2500_LANCE_ADDR_RAP);								/* Select CSR2 */
	writew(((u32) &init_block & 0xff0000) >> 16, CISCO2500_LANCE_ADDR_RDP);				/* Initialisation Block Upper Address */

	// Set byte swap
	writew(0x3, CISCO2500_LANCE_ADDR_RAP);								/* Select CSR3 */
	writew(LANCE_AM79C90_CSR3_BSWP, CISCO2500_LANCE_ADDR_RDP);

	// Initialise LANCE
	writew(0x0, CISCO2500_LANCE_ADDR_RAP);								/* Select CSR0 */
	writew(LANCE_AM79C90_CSR0_CTRL_INIT, CISCO2500_LANCE_ADDR_RDP);					/* LANCE initialise */
	//writew(LANCE_AM79C90_CSR0_CTRL_INEA, CISCO2500_LANCE_ADDR_RDP);					/* Enable interrupts */

	// Wait for interrupt signalling initialisation complete
	now = get_timer(0);
	while ((get_timer(now) < 1000) && !lance_initialised) {
		if (readw(CISCO2500_LANCE_ADDR_RDP) & LANCE_AM79C90_CSR0_STS_IDON) lance_initialised = 1;
		udelay(5);
	}

	if (!lance_initialised) {
		printf("Failed to initialise LANCE.\n");
		goto error;
	}

	printf("LANCE initialised!!!\n");

error:
	// Remove interrupt handler
	free_hdlr(EVA_INTERRUPT_LEVEL4);

	printf ("\n\n");
	return (0);
}

// Check address alignment, and that address is only 24 bits
bool check_addr(void *ptr, unsigned int check_bits) {
	if (((u32) ptr & check_bits) || ((u32) ptr & 0xff000000)) {
		return false;
	} else {
		return true;
	}
}

void lance_irq_handler(void *not_used) {
	u16	flags, lance_status;

	__asm__ __volatile__("nop");

//	/* Check if LANCE is source */
//	writew(0x0, CISCO2500_LANCE_ADDR_RAP);							/* Select CSR0 */
//	lance_status = readw(CISCO2500_LANCE_ADDR_RDP);
//	if (lance_status & LANCE_AM79C90_CSR0_STS_INTR) {
//		// Check for error
//		if (lance_status & LANCE_AM79C90_CSR0_STS_ERROR) {
//		} else {
//			if (lance_status & LANCE_AM79C90_CSR0_STS_IDON) lance_initialised = 1;
//
//			/* Clear interrupt */
//			flags = LANCE_AM79C90_CSR0_STS_RINT | LANCE_AM79C90_CSR0_STS_TINT | LANCE_AM79C90_CSR0_STS_IDON | LANCE_AM79C90_CSR0_CTRL_INEA;
//			writew(flags, CISCO2500_LANCE_ADDR_RDP);				/* Clear IDON */
//		}
//	}

//	lance_initialised = 1;
}
