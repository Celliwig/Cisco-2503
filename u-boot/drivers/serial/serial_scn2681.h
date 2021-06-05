/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Celliwig
 */

#ifndef _SERIAL_SCN2681_
#define _SERIAL_SCN2681_

/* Information about a serial port */
struct scn2681_serial_plat {
	fdt_addr_t base;			/* address of registers in physical memory */
	unsigned long int clock_rate;
};

/* Register definitions */
#define	SCN2681_REG_MODE		0x0	/* Mode1/Mode2 (R/W) */
#define	SCN2681_REG_STATUS		0x1	/* Status (R) */
#define SCN2681_REG_CLK_SELECT		0x1	/* Clock Select (W) */
#define SCN2681_REG_COMMAND		0x2	/* Command (W) */
#define SCN2681_REG_RX			0x3	/* Rx (R) */
#define SCN2681_REG_TX			0x3	/* Tx (W) */
#define SCN2681_REG_AUX_CTRL		0x4	/* Auxiliary Control (W) */

/* Commands */
#define SCN2681_CMD_EN_RX		0x01	/* Command: Enable Receiver */
#define SCN2681_CMD_DSB_RX		0x02	/* Command: Disable Receiver */
#define SCN2681_CMD_EN_TX		0x04	/* Command: Enable Transmitter */
#define SCN2681_CMD_DSB_TX		0x08	/* Command: Disable Transmitter */
#define SCN2681_CMD_RST_MR		0x10	/* Command: Reset Mode Register Pointer */
#define SCN2681_CMD_RST_RX		0x20	/* Command: Reset Receiver */
#define SCN2681_CMD_RST_TX		0x30	/* Command: Reset Transmitter */
#define	SCN2681_CMD_RST_ERR		0x40	/* Command: Reset Error Status */

/* BRG (ACR[7] = 0) */
#define SCN2681_BRG0_50			0x00
#define SCN2681_BRG0_110		0x01
#define SCN2681_BRG0_135		0x02
#define SCN2681_BRG0_200		0x03
#define SCN2681_BRG0_300		0x04
#define SCN2681_BRG0_600		0x05
#define SCN2681_BRG0_1200		0x06
#define SCN2681_BRG0_1050		0x07
#define SCN2681_BRG0_2400		0x08
#define SCN2681_BRG0_4800		0x09
#define SCN2681_BRG0_7200		0x0a
#define SCN2681_BRG0_9600		0x0b
#define SCN2681_BRG0_38400		0x0c

/* BRG (ACR[7] = 1) */
#define SCN2681_BRG1_75			0x00
#define SCN2681_BRG1_110		0x01
#define SCN2681_BRG1_135		0x02
#define SCN2681_BRG1_150		0x03
#define SCN2681_BRG1_300		0x04
#define SCN2681_BRG1_600		0x05
#define SCN2681_BRG1_1200		0x06
#define SCN2681_BRG1_2000		0x07
#define SCN2681_BRG1_2400		0x08
#define SCN2681_BRG1_4800		0x09
#define SCN2681_BRG1_1800		0x0a
#define SCN2681_BRG1_9600		0x0b
#define SCN2681_BRG1_19200		0x0c

#define SCN2681_MODE1_BPC_5		0x00	/* Mode1: Bits Per Character: 5 */
#define SCN2681_MODE1_BPC_6		0x01	/* Mode1: Bits Per Character: 6 */
#define SCN2681_MODE1_BPC_7		0x02	/* Mode1: Bits Per Character: 7 */
#define SCN2681_MODE1_BPC_8		0x03	/* Mode1: Bits Per Character: 8 */
#define SCN2681_MODE1_PARITY_EVEN	0x00	/* Mode1: Parity: Even */
#define SCN2681_MODE1_PARITY_ODD	0x04	/* Mode1: Parity: Odd */
#define	SCN2681_MODE1_PM_WITH		0x00	/* Mode1: Parity Mode: With Parity */
#define	SCN2681_MODE1_PM_FORCE		0x08	/* Mode1: Parity Mode: Force Parity */
#define	SCN2681_MODE1_PM_NO		0x10	/* Mode1: Parity Mode: No Parity */
#define	SCN2681_MODE1_PM_MULTI		0x18	/* Mode1: Parity Mode: Multidrop Mode */
#define SCN2681_MODE1_EM_CHAR		0x00	/* Mode1: Error Mode: Character */
#define SCN2681_MODE1_EM_BLOCK		0x20	/* Mode1: Error Mode: Block */
#define SCN2681_MODE1_RXINT_RXRDY	0x00	/* Mode1: Rx Interrupt Select: RxRDY */
#define SCN2681_MODE1_RXINT_FFULL	0x40	/* Mode1: Rx Interrupt Select: FFULL */
#define SCN2681_MODE1_RXRTS_OFF		0x00	/* Mode1: Rx RTS Control: Off */
#define SCN2681_MODE1_RXRTS_ON		0x80	/* Mode1: Rx RTS Control: On */

#define SCN2681_MODE2_SBL_0563		0x00	/* Mode2: Stop Bit Length: 0.563 */
#define SCN2681_MODE2_SBL_0625		0x01	/* Mode2: Stop Bit Length: 0.625 */
#define SCN2681_MODE2_SBL_0688		0x02	/* Mode2: Stop Bit Length: 0.688 */
#define SCN2681_MODE2_SBL_0750		0x03	/* Mode2: Stop Bit Length: 0.750 */
#define SCN2681_MODE2_SBL_0813		0x04	/* Mode2: Stop Bit Length: 0.813 */
#define SCN2681_MODE2_SBL_0875		0x05	/* Mode2: Stop Bit Length: 0.875 */
#define SCN2681_MODE2_SBL_0938		0x06	/* Mode2: Stop Bit Length: 0.938 */
#define SCN2681_MODE2_SBL_1000		0x07	/* Mode2: Stop Bit Length: 1.000 */
#define SCN2681_MODE2_SBL_1563		0x08	/* Mode2: Stop Bit Length: 1.563 */
#define SCN2681_MODE2_SBL_1625		0x09	/* Mode2: Stop Bit Length: 1.625 */
#define SCN2681_MODE2_SBL_1688		0x0a	/* Mode2: Stop Bit Length: 1.688 */
#define SCN2681_MODE2_SBL_1750		0x0b	/* Mode2: Stop Bit Length: 1.750 */
#define SCN2681_MODE2_SBL_1813		0x0c	/* Mode2: Stop Bit Length: 1.813 */
#define SCN2681_MODE2_SBL_1875		0x0d	/* Mode2: Stop Bit Length: 1.875 */
#define SCN2681_MODE2_SBL_1938		0x0e	/* Mode2: Stop Bit Length: 1.938 */
#define SCN2681_MODE2_SBL_2000		0x0f	/* Mode2: Stop Bit Length: 2.000 */
#define	SCN2681_MODE2_TXCTS_OFF		0x00	/* Mode2: Tx CTS Enable: Off */
#define	SCN2681_MODE2_TXCTS_ON		0x10	/* Mode2: Tx CTS Enable: On */
#define	SCN2681_MODE2_TXRTS_OFF		0x00	/* Mode2: Tx RTS: Off */
#define	SCN2681_MODE2_TXRTS_ON		0x20	/* Mode2: Tx RTS: On */
#define SCN2681_MODE2_CHMODE_NORMAL	0x00	/* Mode2: Channel Mode: Normal */
#define SCN2681_MODE2_CHMODE_ECHO	0x40	/* Mode2: Channel Mode: Auto-Echo */
#define SCN2681_MODE2_CHMODE_LLOOP	0x80	/* Mode2: Channel Mode: Local Loop */
#define SCN2681_MODE2_CHMODE_RLOOP	0xc0	/* Mode2: Channel Mode: Remote Loop */

#define	SCN2681_STATUS_RXRDY		0x01	/* Status Register: Set if Rx buffer contains character */
#define	SCN2681_STATUS_FFULL		0x02	/* Status Register: Set if Rx FIFO full */
#define	SCN2681_STATUS_TXRDY		0x04	/* Status Register: Set if Tx holding register empty */
#define	SCN2681_STATUS_TXEMT		0x08	/* Status Register: Set if no characters to/being transmit */
#define	SCN2681_STATUS_ERR_OVER		0x10	/* Status Register: Set if a new character is received and the FIFO is full */
#define	SCN2681_STATUS_ERR_PARITY	0x20	/* Status Register: Set if a parity error is detected while receiving a character */
#define	SCN2681_STATUS_ERR_FRAME	0x40	/* Status Register: Set if no STOP bit deteced while receiving a character */
#define	SCN2681_STATUS_RCVD_BREAK	0x80	/* Status Register: Set if a BREAK was received */

#endif
