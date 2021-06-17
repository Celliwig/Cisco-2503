/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Celliwig
 */

#define	DEBUG	1

#include <common.h>
#include <dm.h>
#include <net.h>
#include <asm/io.h>

// Need this to enable LANCE
#include "../../board/cisco/2500/cisco-250x.h"

//#include <clock_legacy.h>
//#include <cpu_func.h>
//#include <errno.h>
//#include <log.h>
//#include <miiphy.h>
//#include <malloc.h>
//#include <asm/cache.h>
//#include <asm/global_data.h>
//#include <linux/bitops.h>
//#include <linux/compiler.h>
//#include <linux/delay.h>
//#include <linux/err.h>
//#include <linux/mii.h>
//#include <wait_bit.h>

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
	printf("	MAC: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
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
	iblock->phy_addr1	= (pdata->enetaddr[0] << 8) | pdata->enetaddr[1];		/* Ethernet Address */
	iblock->phy_addr2	= (pdata->enetaddr[2] << 8) | pdata->enetaddr[3];
	iblock->phy_addr3	= (pdata->enetaddr[4] << 8) | pdata->enetaddr[5];
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

static void lance_am79c92_reinitialise(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);

	/* Select CSR0 */
	priv->regs->rap = LANCE_AM79C90_CSR0;
	/* Set STOP */
	priv->regs->rdp = LANCE_AM79C90_CSR0_CTRL_STOP;

	lance_am79c92_setup_init_block(dev);
	lance_am79c92_setup_ring_descriptors(dev);

#ifdef DEBUG
	lance_am79c92_print_init_block(dev);
	lance_am79c92_print_rx_ring_descriptor(dev);
	lance_am79c92_print_tx_ring_descriptor(dev);
#endif
}

/*
 * Ethernet I/O
 */
static int lance_am79c92_eth_send(struct udevice *dev, void *packet, int length)
{
//	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
//	struct ag7xxx_dma_desc *curr;
//	u32 start, end;
//
//	curr = &priv->tx_mac_descrtable[priv->tx_currdescnum];
//
//	/* Cache: Invalidate descriptor. */
//	start = (u32)curr;
//	end = start + sizeof(*curr);
//	invalidate_dcache_range(start, end);
//
//	if (!(curr->config & AG7XXX_DMADESC_IS_EMPTY)) {
//		printf("ag7xxx: Out of TX DMA descriptors!\n");
//		return -EPERM;
//	}
//
//	/* Copy the packet into the data buffer. */
//	memcpy(phys_to_virt(curr->data_addr), packet, length);
//	curr->config = length & AG7XXX_DMADESC_PKT_SIZE_MASK;
//
//	/* Cache: Flush descriptor, Flush buffer. */
//	start = (u32)curr;
//	end = start + sizeof(*curr);
//	flush_dcache_range(start, end);
//	start = (u32)phys_to_virt(curr->data_addr);
//	end = start + length;
//	flush_dcache_range(start, end);
//
//	/* Load the DMA descriptor and start TX DMA. */
//	writel(AG7XXX_ETH_DMA_TX_CTRL_TXE,
//	       priv->regs + AG7XXX_ETH_DMA_TX_CTRL);
//
//	/* Switch to next TX descriptor. */
//	priv->tx_currdescnum = (priv->tx_currdescnum + 1) % CONFIG_TX_DESCR_NUM;

	return 0;
}

static int lance_am79c92_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
//	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
//	struct ag7xxx_dma_desc *curr;
//	u32 start, end, length;
//
//	curr = &priv->rx_mac_descrtable[priv->rx_currdescnum];
//
//	/* Cache: Invalidate descriptor. */
//	start = (u32)curr;
//	end = start + sizeof(*curr);
//	invalidate_dcache_range(start, end);
//
//	/* No packets received. */
//	if (curr->config & AG7XXX_DMADESC_IS_EMPTY)
//		return -EAGAIN;
//
//	length = curr->config & AG7XXX_DMADESC_PKT_SIZE_MASK;
//
//	/* Cache: Invalidate buffer. */
//	start = (u32)phys_to_virt(curr->data_addr);
//	end = start + length;
//	invalidate_dcache_range(start, end);
//
//	/* Receive one packet and return length. */
//	*packetp = phys_to_virt(curr->data_addr);
//	return length;

	return 0;
}

static int lance_am79c92_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
//	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
//	struct ag7xxx_dma_desc *curr;
//	u32 start, end;
//
//	curr = &priv->rx_mac_descrtable[priv->rx_currdescnum];
//
//	curr->config = AG7XXX_DMADESC_IS_EMPTY;
//
//	/* Cache: Flush descriptor. */
//	start = (u32)curr;
//	end = start + sizeof(*curr);
//	flush_dcache_range(start, end);
//
//	/* Switch to next RX descriptor. */
//	priv->rx_currdescnum = (priv->rx_currdescnum + 1) % CONFIG_RX_DESCR_NUM;

	return 0;
}

static int lance_am79c92_eth_start(struct udevice *dev)
{
//	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);
//
//	/* FIXME: Check if link up */
//
//	/* Clear the DMA rings. */
//	ag7xxx_dma_clean_tx(dev);
//	ag7xxx_dma_clean_rx(dev);
//
//	/* Load DMA descriptors and start the RX DMA. */
//	writel(virt_to_phys(&priv->tx_mac_descrtable[priv->tx_currdescnum]),
//	       priv->regs + AG7XXX_ETH_DMA_TX_DESC);
//	writel(virt_to_phys(&priv->rx_mac_descrtable[priv->rx_currdescnum]),
//	       priv->regs + AG7XXX_ETH_DMA_RX_DESC);
//	writel(AG7XXX_ETH_DMA_RX_CTRL_RXE,
//	       priv->regs + AG7XXX_ETH_DMA_RX_CTRL);

	return 0;
}

static void lance_am79c92_eth_stop(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);

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
	void __iomem *iobase;

	iobase = map_physmem(pdata->iobase, 0x200, MAP_NOCACHE);
	priv->regs = iobase;

	debug("%s, iobase=%p, priv=%p, csr3=%d\n", __func__, iobase, priv, priv->bus_master_mode);

	// Enable LANCE
	// Need to move this to a reset control
	writeb(readb(CISCO2500_REG_SYSCTRL5) & ~CISCO2500_REG_SYSCTRL5_RSTCTRL_LANCE, CISCO2500_REG_SYSCTRL5);

	lance_am79c92_reinitialise(dev);

	return 0;
}

static int lance_am79c92_eth_remove(struct udevice *dev)
{
	struct lance_am79c92_eth_priv *priv = dev_get_priv(dev);

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
