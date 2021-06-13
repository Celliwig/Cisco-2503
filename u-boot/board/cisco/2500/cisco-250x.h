// SPDX-License-Identifier: GPL-2.0+
/*
 * Board definitions for Cisco 250x Router
 *
 * (C) Copyright 2021 Celliwig
 *
 */

/********************************************************************************/
/*			System Control Registers				*/
/********************************************************************************/
#define CISCO2500_REG_SYSCTRL0			0x2110000
#define CISCO2500_REG_SYSCTRL1			0x2110001
#define CISCO2500_REG_SYSCTRL2			0x2110002				/* Hardware Revision */
#define CISCO2500_REG_SYSCTRL3			0x2110003
#define CISCO2500_REG_SYSCTRL4			0x2110004
#define CISCO2500_REG_SYSCTRL5			0x2110005				/* Reset Control */
#define CISCO2500_REG_SYSCTRL6			0x2110006
#define CISCO2500_REG_SYSCTRL7			0x2110007				/* IRQ Enable */

/********************************************************************************/
/*			System Control Register Assignments			*/
/********************************************************************************/
#define CISCO2500_REG_SYSCTRL1_BROM_RMAP	0x01					/* Boot ROM remap */
#define CISCO2500_REG_SYSCTRL1_RAM_PARITY	0x08					/* Enable RAM parity check */
#define CISCO2500_REG_SYSCTRL1_STATUS_LED	0x10					/* Enable status LED */

#define CISCO2500_REG_SYSCTRL3_BROM_BW16	0x01					/* Boot ROM bus width: 16 */
#define CISCO2500_REG_SYSCTRL3_BROM_BW32	0x02					/* Boot ROM bus width: 32 */
#define CISCO2500_REG_SYSCTRL3_BROM_SZ_1M	0x08					/* Boot ROM size: 1M */
#define CISCO2500_REG_SYSCTRL3_BROM_SZ_128K	0x04					/* Boot ROM size: 128k */
#define CISCO2500_REG_SYSCTRL3_BROM_SZ_256K	0x00					/* Boot ROM size: 256k */
#define CISCO2500_REG_SYSCTRL3_BROM_SZ_512K	0x0C					/* Boot ROM size: 512k */
#define CISCO2500_REG_SYSCTRL3_DRAM_SZ_8M	0x30					/* DRAM size: 8M */
#define CISCO2500_REG_SYSCTRL3_DRAM_SZ_2M	0x20					/* DRAM size: 2M */
#define CISCO2500_REG_SYSCTRL3_DRAM_SZ_4M	0x10					/* DRAM size: 4M */
#define CISCO2500_REG_SYSCTRL3_DRAM_SZ_16M	0x00					/* DRAM size: 16M */

#define CISCO2500_REG_SYSCTRL5_RSTCTRL_LANCE	0x02					/* Reset Control: LANCE ethernet controller */
#define CISCO2500_REG_SYSCTRL5_RSTCTRL_SCA	0x04					/* Reset Control: Serial Communication Adapter (HD64570) */
#define CISCO2500_REG_SYSCTRL5_RSTCTRL_68302	0x08					/* Reset Control: 68302 */

#define CISCO2500_REG_SYSCTRL7_IRQEN_LANCE	0x02					/* IRQ Enable: LANCE Ethernet controller */
#define CISCO2500_REG_SYSCTRL7_IRQEN_SCA	0x04					/* IRQ Enable: Serial Communication Adapter (HD64570) */
#define CISCO2500_REG_SYSCTRL7_IRQEN_68302	0x08					/* IRQ Enable: 68302 */

/********************************************************************************/
/*			Timer Registers						*/
/********************************************************************************/
#define	CISCO2500_REG_TIMER_CTL			0x2120040
#define	CISCO2500_REG_WDOG_RST			0x2120050
#define	CISCO2500_REG_TIMER_VAL			0x2120070

/********************************************************************************/
/*			LANCE Ethernet Controller				*/
/********************************************************************************/
#define CISCO2500_LANCE_ADDR_RDP		0x02130000
#define CISCO2500_LANCE_ADDR_RAP		0x02130002
