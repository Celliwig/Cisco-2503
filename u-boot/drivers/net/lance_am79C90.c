/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Celliwig
 *
 * Based on ag7xxx.c
 */

//#define	DEBUG	1

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <net.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "lance_am79C90.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Internal functions
 */
static void lance_am79c92_print_init_block(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	char *ptr, size;
	unsigned int addr;

	printf("Initialisation Block: 0x%p\n", &priv->init_block);
	printf("	Mode: 0x%04x\n", priv->init_block.mode);
	ptr = (char *) &priv->init_block.phy_addr1;
	printf("	MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", ptr[0] & 0xff, ptr[1] & 0xff, ptr[2] & 0xff, ptr[3] & 0xff, ptr[4] & 0xff, ptr[5] & 0xff);
	addr = ((priv->init_block.rdr_ptr2 & 0x00ff) << 16) | priv->init_block.rdr_ptr1;
	size = (priv->init_block.rdr_ptr2 & 0xff00) >> 13;
	printf("	Rx Ring Ptr: 0x%08x			Rx Ring Size: 0x%02x\n", addr, size);
	addr = ((priv->init_block.tdr_ptr2 & 0x00ff) << 16) | priv->init_block.tdr_ptr1;
	size = (priv->init_block.tdr_ptr2 & 0xff00) >> 13;
	printf("	Tx Ring Ptr: 0x%08x			Tx Ring Size: 0x%02x\n", addr, size);
}

static void lance_am79c92_setup_init_block(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_init_block *iblock = &priv->init_block;
	phys_addr_t rx_ringd_ptr = (phys_addr_t) &priv->rx_ringd;
	phys_addr_t tx_ringd_ptr = (phys_addr_t) &priv->tx_ringd;

	// Configure initialisation block
	iblock->mode		= 0x0000;
	iblock->phy_addr1	= (pdata->enetaddr[1] << 8) | pdata->enetaddr[0];		/* Ethernet Address */
	iblock->phy_addr2	= (pdata->enetaddr[3] << 8) | pdata->enetaddr[2];
	iblock->phy_addr3	= (pdata->enetaddr[5] << 8) | pdata->enetaddr[4];
	iblock->laddr_filter1	= 0x0000;                                                       /* Disable Logical Address Filter */
	iblock->laddr_filter2	= 0x0000;                                                       /* Disables Multicast */
	iblock->laddr_filter3	= 0x0000;
	iblock->laddr_filter4	= 0x0000;
	iblock->rdr_ptr1	= (rx_ringd_ptr & 0xffff);
	iblock->rdr_ptr2	= (((rx_ringd_ptr & 0xff0000) >> 16) | (LANCE_AM79C90_NUM_RINGS_RX_LOG2 << 13));
	iblock->tdr_ptr1	= (tx_ringd_ptr & 0xffff);
	iblock->tdr_ptr2	= (((tx_ringd_ptr & 0xff0000) >> 16) | (LANCE_AM79C90_NUM_RINGS_TX_LOG2 << 13));
}

static void lance_am79c92_print_rx_ring_descriptor(struct udevice *dev)
{
        struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_rx_ring_descript *ringd_ptr = &priv->rx_ringd[0];
	unsigned int i, buf_addr, buf_len, buf_sts, msg_len;

	printf("Rx Ring Descriptor: %p\n", ringd_ptr);
	for (i = 0; i < LANCE_AM79C90_NUM_BUFFERS_RX; i++) {
		buf_addr = ((ringd_ptr[i].rmd1 & 0x00ff) << 16) | ringd_ptr[i].rmd0;
		buf_sts = (ringd_ptr[i].rmd1 & 0xff00) >> 8;
		buf_len = (~(ringd_ptr[i].rmd2 & 0x0fff) + 1) & 0xfff;
		msg_len = ringd_ptr[i].rmd3 & 0x0fff;
		printf("        0x%p:	Address: 0x%08x		Length: 0x%04x		Status: 0x%02x		Msg Length: 0x%04x\n",
						&ringd_ptr[i], buf_addr, buf_len, buf_sts, msg_len);
	}
}

static void lance_am79c92_print_tx_ring_descriptor(struct udevice *dev)
{
        struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_tx_ring_descript *ringd_ptr = &priv->tx_ringd[0];
	unsigned int i, buf_addr, buf_len, buf_sts, err_sts;

	printf("Tx Ring Descriptor: %p\n", ringd_ptr);
	for (i = 0; i < LANCE_AM79C90_NUM_BUFFERS_TX; i++) {
		buf_addr = ((ringd_ptr[i].tmd1 & 0x00ff) << 16) | ringd_ptr[i].tmd0;
		buf_sts = (ringd_ptr[i].tmd1 & 0xff00) >> 8;
		buf_len = (~(ringd_ptr[i].tmd2 & 0x0fff) + 1) & 0xfff;
		err_sts = ringd_ptr[i].tmd3 & 0x0fff;
		printf("        0x%p:	Address: 0x%08x		Length: 0x%04x		Status: 0x%02x		Err Status: 0x%04x\n",
						&ringd_ptr[i], buf_addr, buf_len, buf_sts, err_sts);
	}
}

static void lance_am79c92_setup_ring_descriptors(struct udevice *dev)
{
        struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	unsigned int i, buffer_addr;

	// Setup Rx Ring
	for (i = 0; i < LANCE_AM79C90_NUM_BUFFERS_RX; i++) {
		buffer_addr = (phys_addr_t) &priv->rx_buffer[0] + (i * PKTSIZE);
		priv->rx_ringd[i].rmd0 = buffer_addr & 0xffff;						/* Receive Buffer Pointer LSW */
		priv->rx_ringd[i].rmd1 = LANCE_AM79C90_RMD1_OWN | ((buffer_addr & 0xff0000) >> 16);	/* Receive Buffer Pointer MSB (Owner: LANCE) */
		priv->rx_ringd[i].rmd2 = 0xf000 | ~(PKTSIZE - 1);					/* Maximum Receive Buffer Size */
		priv->rx_ringd[i].rmd3 = 0x0000;							/* Received Packet Size */
	}

	// Setup Tx Ring
	for (i = 0; i < LANCE_AM79C90_NUM_BUFFERS_TX; i++) {
		buffer_addr = (phys_addr_t) &priv->tx_buffer[0] + (i * PKTSIZE);
		priv->tx_ringd[i].tmd0 = buffer_addr & 0xffff;						/* Transmit Buffer Pointer LSW */
		priv->tx_ringd[i].tmd1 = ((buffer_addr & 0xff0000) >> 16);				/* Transmit Buffer Pointer MSB (Owner: Host) */
		priv->tx_ringd[i].tmd2 = 0xf000 | ~(PKTSIZE - 1);					/* Transmit Packet Size */
		priv->tx_ringd[i].tmd3 = 0x0000;							/* Transmit Packet Status */
	}
}

static void lance_am79c92_print_CSRs(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	unsigned short csr_addr = priv->regs->rap;

	priv->regs->rap = LANCE_AM79C90_CSR0;
	printf("LANCE:	CSRO: 0x%04x",  priv->regs->rdp);
	priv->regs->rap = LANCE_AM79C90_CSR1;
	printf("	CSR1: 0x%04x",  priv->regs->rdp);
	priv->regs->rap = LANCE_AM79C90_CSR2;
	printf("	CSR2: 0x%04x",  priv->regs->rdp);
	priv->regs->rap = LANCE_AM79C90_CSR3;
	printf("	CSR3: 0x%04x",  priv->regs->rdp);
	printf("\n");

	// Restore previous CSR address
	priv->regs->rap = csr_addr;
}

static int lance_am79c92_reinitialise(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	phys_addr_t ib_ptr = (phys_addr_t) &priv->init_block;
	unsigned long timeout;

	/* Select CSR0 */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	/* Set STOP */
	priv->regs->rdp = LANCE_AM79C90_CSR0_CTRL_STOP;

	lance_am79c92_setup_init_block(dev);
	lance_am79c92_setup_ring_descriptors(dev);

	/* Set LANCE initialisation address */
	/* Initialisation Block Lower Address */
	priv->regs->rap = LANCE_AM79C90_CSR1;
	priv->regs->rdp = ib_ptr & 0xffff;
	/* Initialisation Block Upper Address */
	priv->regs->rap = LANCE_AM79C90_CSR2;
	priv->regs->rdp = (ib_ptr & 0xff0000) >> 16;
	/* Bus Master Mode */
	priv->regs->rap = LANCE_AM79C90_CSR3;
	priv->regs->rdp = priv->bus_master_mode;

#ifdef DEBUG
	lance_am79c92_print_init_block(dev);
	lance_am79c92_print_rx_ring_descriptor(dev);
	lance_am79c92_print_tx_ring_descriptor(dev);
	lance_am79c92_print_CSRs(dev);
#endif

	priv->rx_ringd_index = 0;
	priv->tx_ringd_index = 0;
	priv->initialised = false;
	priv->rxon = false;
	priv->txon = false;

	// Initialise LANCE
	priv->regs->rap = LANCE_AM79C90_CSR0;
	priv->regs->rdp = LANCE_AM79C90_CSR0_CTRL_INIT;

	// Wait for IDON (Initialisation Done)
	timeout = get_timer(0) + usec2ticks(1000000);
	while ((get_timer(0) < timeout) && !priv->initialised) {
		// Poll status
		if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_IDON) {
			// Clear IDON
			priv->regs->rdp = LANCE_AM79C90_CSR0_STS_IDON;
			priv->initialised = true;
		}
		udelay(50);
	}

	if (!priv->initialised) {
		debug("%s: Failed to initialise.\n", __func__);
		return -ETIMEDOUT;
	}
	debug("%s: Initialised.\n", __func__);

	return 0;
}

static void lance_am79c92_print_packet(struct udevice *dev, bool print_tx)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_rx_ring_descript *rx_ringd_ptr;
	volatile struct lance_am79c92_tx_ring_descript *tx_ringd_ptr;
	phys_addr_t packet_addr;
	char *packet_data;
	short packet_size,display_size;
	unsigned int i;

	if (print_tx) {
		/* Get current Tx ring descriptor */
		tx_ringd_ptr = &priv->tx_ringd[priv->tx_ringd_index];
		packet_addr = ((tx_ringd_ptr->tmd1 & 0x00ff) << 16) | tx_ringd_ptr->tmd0;
		packet_size = -tx_ringd_ptr->tmd2;
	} else {
		/* Get current Rx ring descriptor */
		rx_ringd_ptr = &priv->rx_ringd[priv->rx_ringd_index];
		packet_addr = ((rx_ringd_ptr->rmd1 & 0x00ff) << 16) | rx_ringd_ptr->rmd0;
		packet_size = rx_ringd_ptr->rmd3;
	}
	packet_data = (char *) packet_addr;
	display_size = packet_size <= 64 ? packet_size : 64;

	printf("%s: Packet Address: 0x%08X		Packet Size: %u\n	", __func__, packet_addr, packet_size);
	for (i = 0; i < display_size; i++) {
		if ((i % 0x10) == 0) printf("\n	");
		printf("0x%02X   ", packet_data[i] & 0xff);
	}
	printf("\n");
}

/*
 * Ethernet I/O
 */
/*
 * lance_am79c92_eth_send
 * Queues a packet for transmission.
 * Return: >= 0 is success.
 */
static int lance_am79c92_eth_send(struct udevice *dev, void *packet, int length)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_tx_ring_descript *ringd_ptr;
	phys_addr_t start, end;
	unsigned long timeout;
	char *pad_ptr;
	int rtn = 0;

	debug("%s\n", __func__);

	/* Check Tx status */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	if (!(priv->regs->rdp & LANCE_AM79C90_CSR0_CTRL_TXON)) {
		printf("%s: Tx stopped!		CSR0: %d\n", __func__, priv->regs->rdp);
		return -EIO;
	}

	/* Clear any existing TINT status */
	if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_TINT) priv->regs->rdp = LANCE_AM79C90_CSR0_STS_TINT;

	/* Get current Tx ring descriptor */
	ringd_ptr = &priv->tx_ringd[priv->tx_ringd_index];

	/* Cache: Invalidate descriptor. */
	start = (phys_addr_t) ringd_ptr;
	end = start + sizeof(struct lance_am79c92_tx_ring_descript);
	invalidate_dcache_range(start, end);

	/* Check ring descriptor ownership */
	if (ringd_ptr->tmd1 & LANCE_AM79C90_TMD1_OWN) {
		printf("%s: No Tx buffers available.\n", __func__);
		return -ENOSPC;
	}

	/* Copy the packet into the data buffer. */
	start = ((ringd_ptr->tmd1 & 0x00ff) << 16) | ringd_ptr->tmd0;
	memcpy(phys_to_virt(start), packet, length);
	/* Need to pad buffer to minimum size (erasing previous data) */
	pad_ptr = (char *) start + length;
	while (length < MINIMUM_PACKET_LEN) {
		pad_ptr = '\0';
		pad_ptr++;
		length++;
	}
	/* Set packet length in ring descriptor */
	ringd_ptr->tmd2 = ~(length - 1);

	/* Cache: Flush buffer, Flush descriptor */
	start = (phys_addr_t) phys_to_virt(((ringd_ptr->tmd1 & 0x00ff) << 16) | ringd_ptr->tmd0);
	end = start + length;
	flush_dcache_range(start, end);
	start = (phys_addr_t) ringd_ptr;
	end = start + sizeof(struct lance_am79c92_tx_ring_descript);
	flush_dcache_range(start, end);

#ifdef DEBUG
	lance_am79c92_print_packet(dev, true);
#endif

	/* Set start/end packet, and release descriptor ownership */
	ringd_ptr->tmd1 = LANCE_AM79C90_TMD1_OWN | LANCE_AM79C90_TMD1_STP | LANCE_AM79C90_TMD1_ENP | (ringd_ptr->tmd1 & 0x00ff);

	/* Wait for transmission to complete */
	timeout = get_timer(0) + usec2ticks(50000);
	while (true) {
		/* Cache: Flush descriptor */
		flush_dcache_range(start, end);

		/* Check for timeout */
		if (get_timer(0) < timeout) {
			/* Check ring status */
			if (!(ringd_ptr->tmd1 & LANCE_AM79C90_TMD1_OWN)) break;
		} else {
			printf("%s: Timeout sending packet.\n", __func__);
			rtn = -ETIMEDOUT;
			break;
		}
		udelay(50);
	}

	/* Check CSR0 for errors */
	if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_ERROR) {
		/* Babble Error */
		if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_BABBLE) {
			printf("%s: Babble error.\n", __func__);
			priv->regs->rdp = LANCE_AM79C90_CSR0_STS_BABBLE;
			rtn = -ETIMEDOUT;
		}
		/* Collision Error */
		if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_CERR) {
			printf("%s: Collision error.\n", __func__);
			priv->regs->rdp = LANCE_AM79C90_CSR0_STS_CERR;
			rtn = -EREMOTEIO;
		}
		/* Bus Master Error */
		if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_MERR) {
			printf("%s: Bus Master error.\n", __func__);
			// No point clearing the error, LANCE needs to be reset
			rtn = -EIO;
		}
	}

	/* Check packet status */
	if (ringd_ptr->tmd1 & LANCE_AM79C90_TMD1_ERR) {
		/* Underflow Error */
		if (ringd_ptr->tmd3 & LANCE_AM79C90_TMD3_UFLO) {
			printf("%s: Underflow error.\n", __func__);
			rtn = -EIO;
		}
		/* Late Collision Error */
		if (ringd_ptr->tmd3 & LANCE_AM79C90_TMD3_LCOL) {
			printf("%s: Late collision error.\n", __func__);
			rtn = -EREMOTEIO;
		}
		/* Loss Of Carrier Error */
		if (ringd_ptr->tmd3 & LANCE_AM79C90_TMD3_LCAR) {
			printf("%s: Loss of carrier error.\n", __func__);
			rtn = -EREMOTEIO;
		}
		/* Retry Error */
		if (ringd_ptr->tmd3 & LANCE_AM79C90_TMD3_RTRY) {
			printf("%s: Retry error.\n", __func__);
			rtn = -EREMOTEIO;
		}
	}

	/* Switch to next TX descriptor. */
	priv->tx_ringd_index = (priv->tx_ringd_index + 1) % LANCE_AM79C90_NUM_BUFFERS_TX;

	return rtn;
}

/*
 * lance_am79c92_eth_recv
 * Fetches a received packet, if one exists.
 * Return: >0, it's the packet length.
 *          0, error and clean up packet.
 *         <0, error.
 */
static int lance_am79c92_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_rx_ring_descript *ringd_ptr;
	phys_addr_t start, end;
	unsigned int length;

	debug("%s\n", __func__);

        /* Check Rx status */
        priv->regs->rap = LANCE_AM79C90_CSR0;
        if (!(priv->regs->rdp & LANCE_AM79C90_CSR0_CTRL_RXON)) {
                printf("%s: Rx stopped!         CSR0: %d\n", __func__, priv->regs->rdp);
                return -EIO;
        }

        /* Clear any existing RINT status */
        if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_RINT) priv->regs->rdp = LANCE_AM79C90_CSR0_STS_RINT;

	/* Get current Rx ring descriptor */
	ringd_ptr = &priv->rx_ringd[priv->rx_ringd_index];

	/* Cache: Invalidate descriptor. */
	start = (phys_addr_t) ringd_ptr;
	end = start + sizeof(struct lance_am79c92_rx_ring_descript);
	invalidate_dcache_range(start, end);

	/* No packets received. */
	if (ringd_ptr->rmd1 & LANCE_AM79C90_RMD1_OWN) {
		debug("%s: No packet received.\n", __func__);
		return -EAGAIN;
	}

	/* Check packet status */
	if (ringd_ptr->rmd1 & LANCE_AM79C90_RMD1_ERR) {
		/* Framing Error */
		if (ringd_ptr->rmd1 & LANCE_AM79C90_RMD1_FRAM) {
			printf("%s: Framing Error.\n", __func__);
			return 0;
		}
		/* Overflow Error */
		if (ringd_ptr->rmd1 & LANCE_AM79C90_RMD1_OFLO) {
			printf("%s: Overflow Error.\n", __func__);
			return 0;
		}
		/* CRC Error */
		if (ringd_ptr->rmd1 & LANCE_AM79C90_RMD1_CRC) {
			printf("%s: CRC Error.\n", __func__);
			return 0;
		}
		/* Buffer Error */
		if (ringd_ptr->rmd1 & LANCE_AM79C90_RMD1_BUFF) {
			printf("%s: Buffer Error.\n", __func__);
			return 0;
		}
	}

	/* Get received packet length */
	length = ringd_ptr->rmd3;

	/* Cache: Invalidate buffer. */
	start = (phys_addr_t) phys_to_virt(((ringd_ptr->rmd1 & 0x00ff) << 16) | ringd_ptr->rmd0);
	end = start + length;
	invalidate_dcache_range(start, end);

#ifdef DEBUG
	lance_am79c92_print_packet(dev, false);
#endif

	/* Receive one packet and return length. */
	*packetp = (uchar *) phys_to_virt(start);
	return length;
}

static int lance_am79c92_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	volatile struct lance_am79c92_rx_ring_descript *ringd_ptr;
	phys_addr_t start, end;

	debug("%s\n", __func__);

	/* Check MISS status */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	if (priv->regs->rdp & LANCE_AM79C90_CSR0_STS_MISS) {
		priv->regs->rdp = LANCE_AM79C90_CSR0_STS_MISS;
		printf("%s: Missed Packet.\n", __func__);
	}

	/* Get current Rx ring descriptor */
	ringd_ptr = &priv->rx_ringd[priv->rx_ringd_index];

	/* Clear message length */
	ringd_ptr->rmd3 = 0x0;
	/* Release ownership */
	ringd_ptr->rmd1 = LANCE_AM79C90_RMD1_OWN | (ringd_ptr->rmd1 & 0x00ff);

	/* Cache: Flush descriptor. */
	start = (phys_addr_t) ringd_ptr;
	end = start + sizeof(struct lance_am79c92_rx_ring_descript);
	flush_dcache_range(start, end);

	/* Switch to next RX descriptor. */
	priv->rx_ringd_index = (priv->rx_ringd_index + 1) % LANCE_AM79C90_NUM_BUFFERS_RX;

	return 0;
}

static int lance_am79c92_eth_start(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	unsigned long timeout;
	int rtn;

	debug("%s\n", __func__);

	/* FIXME: Check if link up */

	rtn = lance_am79c92_reinitialise(dev);
	if (rtn) return rtn;

	/* Select CSR0 */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	/* Set START */
	priv->regs->rdp = LANCE_AM79C90_CSR0_CTRL_STRT;

	// Wait for RXON & TXON
	timeout = get_timer(0) + usec2ticks(1000000);
	while ((get_timer(0) < timeout) && !priv->rxon && !priv->txon) {
		// Poll status
		if (priv->regs->rdp & LANCE_AM79C90_CSR0_CTRL_RXON) {
			priv->rxon = true;
		}
		if (priv->regs->rdp & LANCE_AM79C90_CSR0_CTRL_TXON) {
			priv->txon = true;
		}
		udelay(50);
	}

	if (!priv->rxon && !priv->txon) {
		debug("%s: Failed to start.\n", __func__);
		return -ETIMEDOUT;
	}
	debug("%s: Started.\n", __func__);

	return 0;
}

static void lance_am79c92_eth_stop(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);

	debug("%s\n", __func__);

	/* Select CSR0 */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	/* Set STOP */
	priv->regs->rdp = LANCE_AM79C90_CSR0_CTRL_STOP;
}

/*
 * Hardware setup
 */
static int lance_am79c92_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
	struct reset_ctl reset_lance;
	void __iomem *iobase;
	int err;

	err = reset_get_by_index(dev, 0, &reset_lance);
	if (!err)
		reset_deassert(&reset_lance);

	iobase = map_physmem(pdata->iobase, 0x200, MAP_NOCACHE);
	priv->regs = iobase;

	debug("%s: iobase=%p, priv=%p, csr3=%d\n", __func__, iobase, priv, priv->bus_master_mode);

	priv->rx_ringd_index = 0;
	priv->tx_ringd_index = 0;
	priv->initialised = false;
	priv->rxon = false;
	priv->txon = false;

	return lance_am79c92_reinitialise(dev);
}

static int lance_am79c92_eth_remove(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);

	debug("%s\n", __func__);

	/* Select CSR0 */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	/* Set STOP */
	priv->regs->rdp = LANCE_AM79C90_CSR0_CTRL_STOP;

	return 0;
}

// Check address alignment, and that address is only 24 bits
static bool lance_am79c92_check_addr(phys_addr_t addr, unsigned char check_bits) {
#if defined(LANCE_79C92_QUIRK_CISCO2500)
	// The LANCE on the Cisco doesn't initialise correctly when the RAM
	// objects are located below 2MB
	if ((addr & check_bits) || (addr & 0xff000000) || (addr < 0x00200000)) {
#else
	if ((addr & check_bits) || (addr & 0xff000000)) {
#endif
		return false;
	} else {
		return true;
	}
}

static int lance_am79c92_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);

	// Check initialisation block alignment
	if (!lance_am79c92_check_addr((phys_addr_t) &priv->init_block, 0x1)) return -EINVAL;
	// Rx message descriptor alignment
	if (!lance_am79c92_check_addr((phys_addr_t) &priv->rx_ringd, 0x7)) return -EINVAL;
	// Tx message descriptor alignment
	if (!lance_am79c92_check_addr((phys_addr_t) &priv->tx_ringd, 0x7)) return -EINVAL;

	/* Get LANCE address from FDT */
	pdata->iobase = dev_read_addr(dev);
	pdata->phy_interface = -1;
	/* Default to 10Mbit/s */
	pdata->max_speed = 10;

	priv->bus_master_mode = 0;
	if (dev_read_bool(dev, "ale-control")) priv->bus_master_mode |= LANCE_AM79C90_CSR3_ACON;
	if (dev_read_bool(dev, "byte-control")) priv->bus_master_mode |= LANCE_AM79C90_CSR3_BCON;
	if (dev_read_bool(dev, "byte-swap")) priv->bus_master_mode |= LANCE_AM79C90_CSR3_BSWP;

	return 0;
}

static const struct eth_ops lance_am79c92_eth_ops = {
	.start			= lance_am79c92_eth_start,
	.send			= lance_am79c92_eth_send,
	.recv			= lance_am79c92_eth_recv,
	.free_pkt		= lance_am79c92_eth_free_pkt,
	.stop			= lance_am79c92_eth_stop,
};

static const struct udevice_id lance_am79c92_eth_ids[] = {
	{ .compatible = "amd,79c92" },
	{ }
};

U_BOOT_DRIVER(eth_am79c92) = {
	.name		= "eth_am79c92",
	.id		= UCLASS_ETH,
	.of_match	= lance_am79c92_eth_ids,
	.of_to_plat	= lance_am79c92_eth_of_to_plat,
	.probe		= lance_am79c92_eth_probe,
	.remove		= lance_am79c92_eth_remove,
	.ops		= &lance_am79c92_eth_ops,
	.priv_auto	= sizeof(struct lance_am79c92_eth_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
