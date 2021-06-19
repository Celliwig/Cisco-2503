/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Celliwig
 */

#ifndef _NET_LANCE_AM79C90_
#define _NET_LANCE_AM79C90_

struct lance_am79c92_regs {
	volatile unsigned short rdp;							/* Register Data Port */
	volatile unsigned short rap;							/* Register Address Port */
};

#define	LANCE_AM79C90_CSR0			0x0000					/* LANCE Controller Status */
#define	LANCE_AM79C90_CSR1			0x0001					/* Initialisation Block Address [15:0] */
#define	LANCE_AM79C90_CSR2			0x0002					/* Initialisation Block Address [23:16] */
#define	LANCE_AM79C90_CSR3			0x0003					/* Misc */

/* CSR0 - Control & Status Registers 0 */
#define	LANCE_AM79C90_CSR0_STS_ERROR		1<<15					/* Status: Error */
#define	LANCE_AM79C90_CSR0_STS_BABBLE		1<<14					/* Status: Babble */
#define	LANCE_AM79C90_CSR0_STS_CERR		1<<13					/* Status: Collision Error */
#define	LANCE_AM79C90_CSR0_STS_MISS		1<<12					/* Status: Missed Packet */
#define	LANCE_AM79C90_CSR0_STS_MERR		1<<11					/* Status: Memory Error */
#define	LANCE_AM79C90_CSR0_STS_RINT		1<<10					/* Status: Rx Interrupt */
#define	LANCE_AM79C90_CSR0_STS_TINT		1<<9					/* Status: Tx Interrupt */
#define	LANCE_AM79C90_CSR0_STS_IDON		1<<8					/* Status: Initialisation Done */
#define	LANCE_AM79C90_CSR0_STS_INTR		1<<7					/* Status: Interrupt Flag */
#define	LANCE_AM79C90_CSR0_CTRL_INEA		1<<6					/* Control: Interrupt Enable */
#define	LANCE_AM79C90_CSR0_CTRL_RXON		1<<5					/* Control: Receiver On */
#define	LANCE_AM79C90_CSR0_CTRL_TXON		1<<4					/* Control: Transmitter On */
#define	LANCE_AM79C90_CSR0_CTRL_TDMD		1<<3					/* Control: Transmit Demand */
#define	LANCE_AM79C90_CSR0_CTRL_STOP		1<<2					/* Control: Stop */
#define	LANCE_AM79C90_CSR0_CTRL_STRT		1<<1					/* Control: Start */
#define	LANCE_AM79C90_CSR0_CTRL_INIT		1<<0					/* Control: Initialise */

/* CSR1 - Control & Status Registers 1 */
/* Lower 16 bits of initialisation block address */

/* CSR2 - Control & Status Registers 2 */
/* Upper 8 bits of initialisation block address */

/* CSR3 - Control & Status Registers 3 */
#define LANCE_AM79C90_CSR3_BCON			1<<0					/* Byte Control */
#define LANCE_AM79C90_CSR3_ACON			1<<1					/* ALE Control */
#define LANCE_AM79C90_CSR3_BSWP			1<<2					/* Byte Swap */

struct lance_am79c92_init_block {
	u16	mode;
	u16	phy_addr1;								/* Physical Address */
	u16	phy_addr2;
	u16	phy_addr3;
	u16	laddr_filter1;								/* Logical Address Filter */
	u16	laddr_filter2;
	u16	laddr_filter3;
	u16	laddr_filter4;
	u16	rdr_ptr1;								/* Receive Descriptor Ring Pointer */
	u16	rdr_ptr2;
	u16	tdr_ptr1;								/* Transmit Descriptor Ring Pointer */
	u16	tdr_ptr2;
};

/* Initialisation Block - Mode Bits */
#define LANCE_AM79C90_INIT_MODE_PROM		1<<15					/* Mode: Promiscous */
#define LANCE_AM79C90_INIT_MODE_EMBA		1<<7					/* Mode: Enable Modified Back-Off Algorithm */
#define LANCE_AM79C90_INIT_MODE_INTL		1<<6					/* Mode: Internal Loopback */
#define LANCE_AM79C90_INIT_MODE_DRTY		1<<5					/* Mode: Disable Retry */
#define LANCE_AM79C90_INIT_MODE_COLL		1<<4					/* Mode: Force Collision */
#define LANCE_AM79C90_INIT_MODE_DTCR		1<<3					/* Mode: Disable Transmit CRC */
#define LANCE_AM79C90_INIT_MODE_LOOP		1<<2					/* Mode: Loopback */
#define LANCE_AM79C90_INIT_MODE_DTX		1<<1					/* Mode: Disable Transmitter */
#define LANCE_AM79C90_INIT_MODE_DRX		1<<0					/* Mode: Disable Receiver */

/* Descriptor Ring Size */
#define	LANCE_AM79C90_DRING_SZ1			0x00					/* Descriptor Ring: 1 entry */
#define	LANCE_AM79C90_DRING_SZ2			0x01					/* Descriptor Ring: 2 entry */
#define	LANCE_AM79C90_DRING_SZ4			0x02					/* Descriptor Ring: 4 entry */
#define	LANCE_AM79C90_DRING_SZ8			0x03					/* Descriptor Ring: 8 entry */
#define	LANCE_AM79C90_DRING_SZ16		0x04					/* Descriptor Ring: 16 entry */
#define	LANCE_AM79C90_DRING_SZ32		0x05					/* Descriptor Ring: 32 entry */
#define	LANCE_AM79C90_DRING_SZ64		0x06					/* Descriptor Ring: 64 entry */
#define	LANCE_AM79C90_DRING_SZ128		0x07					/* Descriptor Ring: 128 entry */

struct lance_am79c92_rx_ring_descript {
	volatile u16	rmd0;
	volatile u16	rmd1;
	volatile u16	rmd2;
	volatile u16	rmd3;
};

/* Receive Message Descriptor 1 - Status Bits */
#define LANCE_AM79C90_RMD1_OWN			1<<15					/* Receive Message Descriptor: Owned (0 = Host / 1 = LANCE) */
#define LANCE_AM79C90_RMD1_ERR			1<<14					/* Receive Message Descriptor: Error Summary */
#define LANCE_AM79C90_RMD1_FRAM			1<<13					/* Receive Message Descriptor: Framing Error */
#define LANCE_AM79C90_RMD1_OFLO			1<<12					/* Receive Message Descriptor: Overflow Error */
#define LANCE_AM79C90_RMD1_CRC			1<<11					/* Receive Message Descriptor: CRC Error */
#define LANCE_AM79C90_RMD1_BUFF			1<<10					/* Receive Message Descriptor: Buffer Error */
#define LANCE_AM79C90_RMD1_STP			1<<9					/* Receive Message Descriptor: Start Of Packet */
#define LANCE_AM79C90_RMD1_ENP			1<<8					/* Receive Message Descriptor: End Of Packet */

struct lance_am79c92_tx_ring_descript {
	volatile u16	tmd0;
	volatile u16	tmd1;
	volatile u16	tmd2;
	volatile u16	tmd3;
};

/* Transmit Message Descriptor 1 - Status Bits */
#define LANCE_AM79C90_TMD1_OWN			1<<15					/* Transmit Message Descriptor: Owned (0 = Host / 1 = LANCE) */
#define LANCE_AM79C90_TMD1_ERR			1<<14					/* Transmit Message Descriptor: Error Summary */
#define LANCE_AM79C90_TMD1_ADD_FCS		1<<13					/* Transmit Message Descriptor: Force CRC Append */
#define LANCE_AM79C90_TMD1_MORE			1<<12					/* Transmit Message Descriptor: Indicates more than 1 retry was needed to transmit packet */
#define LANCE_AM79C90_TMD1_ONE			1<<11					/* Transmit Message Descriptor: Indicates exactly 1 retry was needed to transmit packet */
#define LANCE_AM79C90_TMD1_DEF			1<<10					/* Transmit Message Descriptor: Deferred, indicates LANCE had to defer while trying to transmit packet */
#define LANCE_AM79C90_TMD1_STP			1<<9					/* Transmit Message Descriptor: Start Of Packet */
#define LANCE_AM79C90_TMD1_ENP			1<<8					/* Transmit Message Descriptor: End Of Packet */

/* Transmit Message Descriptor 3 - Status Bits */
#define LANCE_AM79C90_TMD3_BUFF			1<<15					/* Transmit Message Descriptor: Buffer Error */
#define LANCE_AM79C90_TMD3_UFLO			1<<14					/* Transmit Message Descriptor: Underflow Error */
#define LANCE_AM79C90_TMD3_LCOL			1<<12					/* Transmit Message Descriptor: Late Collision Error */
#define LANCE_AM79C90_TMD3_LCAR			1<<11					/* Transmit Message Descriptor: Loss Of Carrier Error */
#define LANCE_AM79C90_TMD3_RTRY			1<<10					/* Transmit Message Descriptor: Retry Error */

/************************************************************************************************************************************/

#define LANCE_AM79C90_NUM_RINGS_RX_LOG2		LANCE_AM79C90_DRING_SZ4			/* Rx value for Initialisation Block */
#define	LANCE_AM79C90_NUM_BUFFERS_RX		(1 << LANCE_AM79C90_NUM_RINGS_RX_LOG2)	/* Number of Rx rings/buffers */
#define LANCE_AM79C90_NUM_RINGS_TX_LOG2		LANCE_AM79C90_DRING_SZ1			/* Tx value for Initialisation Block */
#define	LANCE_AM79C90_NUM_BUFFERS_TX		(1 << LANCE_AM79C90_NUM_RINGS_TX_LOG2)	/* Number of Tx rings/buffers */

#define	MINIMUM_PACKET_LEN			60					/* Minimum packet length, pad to this value */

struct lance_am79c92_eth_priv {
	volatile struct lance_am79c92_regs		*regs;

	volatile struct lance_am79c92_init_block	init_block	__aligned(1);
	volatile struct lance_am79c92_rx_ring_descript	rx_ringd[LANCE_AM79C90_NUM_BUFFERS_RX]	__aligned(8);
	volatile struct lance_am79c92_tx_ring_descript	tx_ringd[LANCE_AM79C90_NUM_BUFFERS_TX]	__aligned(8);
	unsigned int rx_ringd_index;
	unsigned int tx_ringd_index;
	volatile char rx_buffer[LANCE_AM79C90_NUM_BUFFERS_RX * PKTSIZE];
	volatile char tx_buffer[LANCE_AM79C90_NUM_BUFFERS_TX * PKTSIZE];

	unsigned short	bus_master_mode;

	bool initialised;
	bool rxon;
	bool txon;
};

#endif		/* _NET_LANCE_AM79C90_ */
