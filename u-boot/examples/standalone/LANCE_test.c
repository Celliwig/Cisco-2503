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
void print_ring_descriptor(char *ptr);
void print_struct_info(void);
void lance_irq_handler(void *not_used);

volatile unsigned char rx_buffer[MAXIMUM_ETHERNET_FRAME_SIZE * LANCE_AM79C90_NUM_BUFFERS_RX];
volatile unsigned char tx_buffer[MAXIMUM_ETHERNET_FRAME_SIZE * LANCE_AM79C90_NUM_BUFFERS_TX];
volatile struct lance_rx_ring_descript __attribute__ ((aligned (8))) rx_ringd[LANCE_AM79C90_NUM_BUFFERS_RX];
volatile struct lance_tx_ring_descript __attribute__ ((aligned (8))) tx_ringd[LANCE_AM79C90_NUM_BUFFERS_TX];
volatile struct lance_init_block __attribute__ ((aligned (1))) init_block;
volatile unsigned char lance_initialised;

volatile unsigned char *rx_buffer_ptr;
volatile unsigned char *tx_buffer_ptr;
volatile struct lance_rx_ring_descript *rx_ringd_ptr;
volatile struct lance_tx_ring_descript *tx_ringd_ptr;
volatile struct lance_init_block *init_block_ptr;
volatile struct lance_regs *lance_dev;

int lance_test(int argc, char *const argv[])
{
	char *test_str = "LANCE: Internal loopback Test!!!";
	unsigned int i;
	char *tmp_buffer_ptr;
	short *tmp_ringd_ptr;
	ulong now;

	/* Setup App */
	app_startup(argv);

	printf ("LANCE ethernet controller test\n");
	lance_initialised = 0;

	lance_dev = 0x02130000;
	//init_block_ptr = 0x00206dac;
	init_block_ptr = (u32) &init_block | 0x200000;					// Rx/Tx won't start below this
	rx_ringd_ptr = (u32) &rx_ringd | 0x200000;
	rx_buffer_ptr = (u32) &rx_buffer | 0x200000;
	tx_ringd_ptr = (u32) &tx_ringd | 0x200000;
	tx_buffer_ptr = (u32) &tx_buffer | 0x200000;

	// Enable LANCE
	writeb(readb(CISCO2500_REG_SYSCTRL5) & ~CISCO2500_REG_SYSCTRL5_RSTCTRL_LANCE, CISCO2500_REG_SYSCTRL5);

	if (argc > 1) {
		if (strcmp(argv[1], "show_regs") == 0) {
			print_struct_info();
		}
		if ((strcmp(argv[1], "set_rap") == 0) && (argc == 3)) {
			ulong val = simple_strtoul(argv[2], NULL, 16);
			lance_dev->rap = 0x03 & val;
			printf("Set RAP: %d\n", lance_dev->rap);
		}
		goto clean_up;
	}

	// Setup interrupt handler
	install_hdlr(EVA_INTERRUPT_LEVEL4, &lance_irq_handler, 0);

	//// Enable LANCE IRQ
	//writeb(readb(CISCO2500_REG_SYSCTRL7) & ~CISCO2500_REG_SYSCTRL7_IRQEN_LANCE, CISCO2500_REG_SYSCTRL7);

	lance_dev->rap = LANCE_AM79C90_CSR0;								/* Select CSR0 */
	lance_dev->rdp = LANCE_AM79C90_CSR0_CTRL_STOP;							/* Set STOP */

	// Check initialisation block alignment
	if (!check_addr(init_block_ptr, 0x1)) {
		printf("Initialisation Block must be word aligned and in the lower 24bits of memory!!!\n");
		goto error;
	}

	// Rx message descriptor alignment
	if (!check_addr(rx_ringd_ptr, 0x7)) {
		printf("Rx message descriptor must be quad word aligned and in the lower 24bits of memory!!!\n");
		goto error;
	}

	// Tx message descriptor alignment
	if (!check_addr(tx_ringd_ptr, 0x7)) {
		printf("Tx message descriptor must be quad word aligned and in the lower 24bits of memory!!!\n");
		goto error;
	}

	printf("\nConfiguring Initialisation Block/Descriptor Rings.\n");
	// Configure initialisation block
	//init_block_ptr->mode		= 0x0000;
	/* Internal Loopback Test */
	init_block_ptr->mode		= LANCE_AM79C90_INIT_MODE_PROM | LANCE_AM79C90_INIT_MODE_INTL | LANCE_AM79C90_INIT_MODE_LOOP;
	init_block_ptr->phy_addr1	= 0xe000;							/* Ethernet Address */
	init_block_ptr->phy_addr2	= 0xb91e;
	init_block_ptr->phy_addr3	= 0x9123;
	init_block_ptr->laddr_filter1	= 0x0000;							/* Disable Logical Address Filter */
	init_block_ptr->laddr_filter2	= 0x0000;							/* Disables Multicast */
	init_block_ptr->laddr_filter3	= 0x0000;
	init_block_ptr->laddr_filter4	= 0x0000;
	init_block_ptr->rdr_ptr1	= ((u32) rx_ringd_ptr & 0xffff);
	init_block_ptr->rdr_ptr2	= ((((u32) rx_ringd_ptr & 0xff0000) >> 16) | LANCE_AM79C90_DRSIZE_4);
	init_block_ptr->tdr_ptr1	= ((u32) tx_ringd_ptr & 0xffff);
	init_block_ptr->tdr_ptr2	= ((((u32) tx_ringd_ptr & 0xff0000) >> 16) | LANCE_AM79C90_DRSIZE_4);

	// Configure Rx Message Descriptor
	memset(rx_ringd_ptr, 0x00, sizeof(struct lance_rx_ring_descript) * LANCE_AM79C90_NUM_BUFFERS_RX);
	tmp_ringd_ptr = rx_ringd_ptr;
	for (i = 0; i < LANCE_AM79C90_NUM_BUFFERS_RX; i++) {
		//printf("Configuring Rx Ring: %p\n", tmp_ringd_ptr);
		tmp_buffer_ptr = (u32) &rx_buffer[MAXIMUM_ETHERNET_FRAME_SIZE * i];

		tmp_ringd_ptr[0] = ((u32) tmp_buffer_ptr & 0xffff);						/* Lower 16 bit of Rx buffer address */
		tmp_ringd_ptr[1] = ((((u32) tmp_buffer_ptr & 0xff0000) >> 16) | LANCE_AM79C90_RMD1_OWN);	/* Upper 8 bits of Rx buffer address, mark as belonging to LANCE */
		tmp_ringd_ptr[2] = (0xf000 | ~(MAXIMUM_ETHERNET_FRAME_SIZE - 1));				/* Upper 4 bits must be set, negative 2's complement buffer size */
		tmp_ringd_ptr[3] = 0x0000;									/* Clear message byte count */

		tmp_ringd_ptr += 4;
	}

	// Configure Tx Message Descriptor
	memset(tx_ringd_ptr, 0x00, sizeof(struct lance_tx_ring_descript) * LANCE_AM79C90_NUM_BUFFERS_TX);
	tmp_ringd_ptr = tx_ringd_ptr;
	for (i = 0; i < LANCE_AM79C90_NUM_BUFFERS_TX; i++) {
		//printf("Configuring Tx Ring: %p\n", tmp_ringd_ptr);
		tmp_buffer_ptr = (u32) &tx_buffer[MAXIMUM_ETHERNET_FRAME_SIZE * i];

		tmp_ringd_ptr[0] = ((u32) tmp_buffer_ptr & 0xffff);						/* Lower 16 bit of Tx buffer address */
		tmp_ringd_ptr[1] = (((u32) tmp_buffer_ptr & 0xff0000) >> 16);					/* Upper 8 bits of Tx buffer address, mark as belonging to Host */
		tmp_ringd_ptr[2] = (0xf000 | ~(MAXIMUM_ETHERNET_FRAME_SIZE - 1));				/* Upper 4 bits must be set, negative 2's complement buffer size */
		tmp_ringd_ptr[3] = 0x0000;									/* Clear errors */

		tmp_ringd_ptr += 4;
	}

	// Set LANCE initialisation address
	lance_dev->rap = LANCE_AM79C90_CSR1;
	lance_dev->rdp = (u32) init_block_ptr & 0xffff;							/* Initialisation Block Lower Address */
	lance_dev->rap = LANCE_AM79C90_CSR2;
	lance_dev->rdp = ((u32) init_block_ptr & 0xff0000) >> 16;						/* Initialisation Block Upper Address */

	// Set byte swap
	lance_dev->rap = LANCE_AM79C90_CSR3;
	lance_dev->rdp = LANCE_AM79C90_CSR3_BSWP;

	print_struct_info();

	// Initialise LANCE
	lance_dev->rap = LANCE_AM79C90_CSR0;
	lance_dev->rdp = LANCE_AM79C90_CSR0_CTRL_INEA | LANCE_AM79C90_CSR0_CTRL_INIT;

	// Wait for interrupt signalling initialisation complete
	now = get_timer(0);
	while ((get_timer(now) < 1000) && !lance_initialised) {
		//// Poll status
		//if (lance_dev->rdp & LANCE_AM79C90_CSR0_STS_IDON) {
		//	// Clear IDON
		//	lance_dev->rdp = LANCE_AM79C90_CSR0_STS_IDON;
		//	lance_initialised = 1;
		//}
		udelay(5);
	}

	if (!lance_initialised) {
		printf("Failed to initialise LANCE.\n");
		goto error;
	}

	printf("LANCE initialised!!!\n\n");

	// Start LANCE
	lance_dev->rap = LANCE_AM79C90_CSR0;
	lance_dev->rdp = LANCE_AM79C90_CSR0_CTRL_INEA | LANCE_AM79C90_CSR0_CTRL_STRT;

	if (lance_dev->rdp & LANCE_AM79C90_CSR0_CTRL_STRT) {
		printf("LANCE started!!!\n\n");
	} else {
		printf("Failed to start LANCE.\n");
		goto error;
	}

	// Copy test string into Tx buffer
	for (int i = 0; i < 32; i++) {
		tx_buffer_ptr[i] = test_str[i];
	}

	// Update Tx Ring Descriptor
	/* Write new size */
	tx_ringd_ptr->tmd2 = 0xf000 | ~(30 - 1);
	/* Set as start/end packet, set ownership to LANCE */
	tx_ringd_ptr->tmd1 = tx_ringd_ptr->tmd1 | LANCE_AM79C90_TMD1_OWN | LANCE_AM79C90_TMD1_STP | LANCE_AM79C90_TMD1_ENP;

	udelay(1000000);
	print_struct_info();

error:
	// Remove interrupt handler
	free_hdlr(EVA_INTERRUPT_LEVEL4);

clean_up:
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

/* Print ring descriptor */
void print_ring_descriptor(char *ptr) {
	printf("	");
	for (int i = 0; i < 0x8; i++, ptr++) {
		printf("0x%02x   ", (*ptr & 0xff));
	}
	printf(" \n");
}

/* Print info from the various structures */
void print_struct_info() {
	char *ptr;

	/* Select CSR0 */
	lance_dev->rap = LANCE_AM79C90_CSR0;
	printf("CSR0: %04x\n", lance_dev->rdp);
	lance_dev->rap = LANCE_AM79C90_CSR1;
	printf("CSR1: %04x\n", lance_dev->rdp);
	lance_dev->rap = LANCE_AM79C90_CSR2;
	printf("CSR2: %04x\n", lance_dev->rdp);
	lance_dev->rap = LANCE_AM79C90_CSR3;
	printf("CSR3: %04x\n", lance_dev->rdp);

	printf("\nInitialisation block at: 0x%p\n", init_block_ptr);
	ptr = init_block_ptr;
	printf("	");
	for (int i = 0; i < 0x10; i++, ptr++) {
		printf("0x%02x   ", (*ptr & 0xff));
	}
	printf(" \n");
	printf("	");
	for (int i = 0; i < 0x08; i++, ptr++) {
		printf("0x%02x   ", (*ptr & 0xff));
	}
	printf(" \n");

	printf("Rx Ring Descriptor: 0x%p		Rx Buffer: 0x%p\n", rx_ringd_ptr, rx_buffer_ptr);
	ptr = rx_ringd_ptr;
	for (int i = 0; i < (1 << (init_block_ptr->rdr_ptr2 >> 13)); i++) {
		print_ring_descriptor(ptr);
		ptr += 0x8;
	}
	printf("Tx Ring Descriptor: 0x%p		Tx Buffer: 0x%p\n", tx_ringd_ptr, tx_buffer_ptr);
	ptr = tx_ringd_ptr;
	for (int i = 0; i < (1 << (init_block_ptr->tdr_ptr2 >> 13)); i++) {
		print_ring_descriptor(ptr);
		ptr += 0x8;
	}
	printf(" \n");
}

void lance_irq_handler(void *not_used) {
	u16	flags, lance_status;

	//__asm__ __volatile__("nop");

	/* Check if LANCE is source */
	lance_dev->rap = LANCE_AM79C90_CSR0;
	lance_status = lance_dev->rdp;
	if (lance_status & LANCE_AM79C90_CSR0_STS_INTR) {
		//printf("LANCE: Interrupt.\n");
		// Check for error
		if (lance_status & LANCE_AM79C90_CSR0_STS_ERROR) {
			printf("\nLANCE: Error!\n\n");
		} else {
			if (lance_status & LANCE_AM79C90_CSR0_STS_IDON) lance_initialised = 1;
		}
		/* Clear interrupt */
		flags = LANCE_AM79C90_CSR0_STS_RINT | LANCE_AM79C90_CSR0_STS_TINT | LANCE_AM79C90_CSR0_STS_IDON | LANCE_AM79C90_CSR0_CTRL_INEA;
		/* Clear IDON */
		lance_dev->rdp = flags;
	}
}
