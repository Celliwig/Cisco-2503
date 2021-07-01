// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Celliwig
 */

#define LOG_CATEGORY UCLASS_SERIAL

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <reset.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include "serial_scn2681.h"
#include <dm/device_compat.h>

static void _scn2681_serial_setbrgtest(struct scn2681_serial_plat *plat, bool brg_testmode_wanted)
{
	// Check if BRG Test mode state needs updating
	if (plat->brg_testmode ^ brg_testmode_wanted) {
		// Update BRG Test mode
		readb(plat->base + SCN2681_REG_BRG_TEST);
		// Update saved state
		plat->brg_testmode = brg_testmode_wanted;
	}
}

static void _scn2681_serial_setbrg(struct scn2681_serial_plat *plat, int baudrate)
{
	bool brg_select_alt = false, brg_test = false;
	unsigned char clk_select = 0;

	/* Shutdown UART */
	writeb(SCN2681_CMD_DSB_TX, plat->base + SCN2681_REG_COMMAND_A);					/* Disable Transmitter */
	writeb(SCN2681_CMD_DSB_RX, plat->base + SCN2681_REG_COMMAND_A);					/* Disable Reciever */

	switch (baudrate) {
	case 150:
		clk_select = (SCN2681_BRG1_150 << 4) | SCN2681_BRG1_150;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 300:
		clk_select = (SCN2681_BRG1_300 << 4) | SCN2681_BRG1_300;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 600:
		clk_select = (SCN2681_BRG1_600 << 4) | SCN2681_BRG1_600;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 1200:
		clk_select = (SCN2681_BRG1_1200 << 4) | SCN2681_BRG1_1200;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 2400:
		clk_select = (SCN2681_BRG1_2400 << 4) | SCN2681_BRG1_2400;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 4800:
		clk_select = (SCN2681_BRG1_4800 << 4) | SCN2681_BRG1_4800;
		brg_select_alt = true;
		brg_test = false;
		break;
	default:
	case 9600:
		clk_select = (SCN2681_BRG1_9600 << 4) | SCN2681_BRG1_9600;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 19200:
		clk_select = (SCN2681_BRG1_19200 << 4) | SCN2681_BRG1_19200;
		brg_select_alt = true;
		brg_test = false;
		break;
	case 38400:
		clk_select = (SCN2681_BRG0_38400 << 4) | SCN2681_BRG0_38400;
		brg_select_alt = false;
		brg_test = false;
		break;
	case 57600:
		clk_select = (SCN2681_BRGTST_57600 << 4) | SCN2681_BRGTST_57600;
		brg_select_alt = false;
		brg_test = true;
		break;
	case 115200:
		clk_select = (SCN2681_BRGTST_115200 << 4) | SCN2681_BRGTST_115200;
		brg_select_alt = false;
		brg_test = true;
		break;
	}

	_scn2681_serial_setbrgtest(plat, brg_test);
	if (brg_select_alt) {
		writeb(SCN2681_ACR_BRG_SELECT, plat->base + SCN2681_REG_AUX_CTRL);			/* Baud Rate set 2 */
	} else {
		writeb(0x00, plat->base + SCN2681_REG_AUX_CTRL);					/* Baud Rate set 1 */
	}
	writeb(clk_select, plat->base + SCN2681_REG_CLK_SELECT_A);					/* Set Tx and Rx rates */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), plat->base + SCN2681_REG_COMMAND_A);		/* Enable Transmitter / Receiver */
}

static int scn2681_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);

	_scn2681_serial_setbrg(plat, baudrate);

	return 0;
}

static int scn2681_serial_setconfig(struct udevice *dev, uint serial_config)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);
	fdt_addr_t base = plat->base;
	uint parity = SERIAL_GET_PARITY(serial_config);
	uint bits = SERIAL_GET_BITS(serial_config);
	uint stop = SERIAL_GET_STOP(serial_config);
	unsigned char mode1 = SCN2681_MODE1_BPC_8 | SCN2681_MODE1_RXRTS_ON;

	/*
	 * only parity config is implemented, check if other serial settings
	 * are the default one.
	 */
	if (bits != SERIAL_8_BITS || stop != SERIAL_ONE_STOP)
		return -ENOTSUPP;									/* not supported in driver*/

	/* Shutdown UART */
	writeb(SCN2681_CMD_DSB_TX, base + SCN2681_REG_COMMAND_A);					/* Disable Transmitter */
	writeb(SCN2681_CMD_DSB_RX, base + SCN2681_REG_COMMAND_A);					/* Disable Reciever */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND_A);					/* Reset Mode Register Pointer */

	switch (parity) {
	case SERIAL_PAR_ODD:
		mode1 |= SCN2681_MODE1_PM_WITH | SCN2681_MODE1_PARITY_ODD;
		break;
	case SERIAL_PAR_EVEN:
		mode1 |= SCN2681_MODE1_PM_WITH | SCN2681_MODE1_PARITY_EVEN;
		break;
	case SERIAL_PAR_NONE:
	default:
		mode1 |= SCN2681_MODE1_PM_NO;
		break;
	}
	writeb(mode1, base + SCN2681_REG_MODE_A);							/* Mode 1: 8-bit, No Parity, RTS On */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND_A);			/* Enable Transmitter / Receiver */

	return 0;
}

static int scn2681_serial_getc(struct udevice *dev)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);
	fdt_addr_t base = plat->base;

	unsigned char uart_status = readb(base + SCN2681_REG_STATUS_A);
	if ((uart_status & SCN2681_STATUS_RXRDY) == 0)
		return -EAGAIN;

	if (uart_status & (SCN2681_STATUS_ERR_OVER | SCN2681_STATUS_ERR_PARITY | SCN2681_STATUS_ERR_FRAME)) {
		writeb(SCN2681_CMD_RST_ERR, base + SCN2681_REG_COMMAND_A);
		return -EIO;
	}

	return readb(base + SCN2681_REG_RX_A);
}

static int _scn2681_serial_putc(fdt_addr_t base,
			      const char c)
{
	if ((readb(base + SCN2681_REG_STATUS_A) & SCN2681_STATUS_TXRDY) == 0)
		return -EAGAIN;

	writeb(c, base + SCN2681_REG_TX_A);

	return 0;
}

static int scn2681_serial_putc(struct udevice *dev, const char c)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);

	return _scn2681_serial_putc(plat->base, c);
}

static int scn2681_serial_pending(struct udevice *dev, bool input)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);
	fdt_addr_t base = plat->base;

	if (input)
		return readb(base + SCN2681_REG_STATUS_A) & SCN2681_STATUS_RXRDY ? 1 : 0;
	else
		return readb(base + SCN2681_REG_STATUS_A) & SCN2681_STATUS_TXRDY ? 0 : 1;
}

static void _scn2681_serial_brgtest_reset(fdt_addr_t base)
{
	/*
	 * To access 57600/115200 baudrates, BRG Test mode must be enabled.
	 * However BRG Test mode can only be toggled, it's state can't be directly
	 * read, and only a hardware reset returns it to it's default.
	 * On the Cisco-2500 a system reset DOES NOT reset the DUART...
	 * Here be dragons....
	 *
	 * Configure Channel A to 9600 (it's the same in all modes)
	 * Configure Channel B to either 1200/115200
	 * Configure Channel A & B as local loopback
	 * Start Channel A/B device's Rx/Tx sections
	 * Write a character to both Tx holding registers
	 * Poll ISR, which ever channel finishes first (RxRDY) indicates BRG Test mode state
	 */

	unsigned char isr = 0, mode1, mode2, tmp;
	unsigned int loop = CONFIG_SYS_CLK;								/* Timers probably not running at this point,
													   so need a crude timeout in the order of a
													   few seconds */

	// Reset Channel A
	writeb(SCN2681_CMD_RST_TX, base + SCN2681_REG_COMMAND_A);					/* Reset Transmitter */
	writeb(SCN2681_CMD_RST_RX, base + SCN2681_REG_COMMAND_A);					/* Reset Receiver */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND_A);					/* Reset Mode Register Pointer */

	// Reset Channel B
	writeb(SCN2681_CMD_RST_TX, base + SCN2681_REG_COMMAND_B);					/* Reset Transmitter */
	writeb(SCN2681_CMD_RST_RX, base + SCN2681_REG_COMMAND_B);					/* Reset Receiver */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND_B);					/* Reset Mode Register Pointer */

	/* Baud Rate Set #2 */
	writeb(SCN2681_ACR_BRG_SELECT, base + SCN2681_REG_AUX_CTRL);

	// Configure Channel A
	writeb(((SCN2681_BRG1_9600 << 4) | SCN2681_BRG1_9600), base + SCN2681_REG_CLK_SELECT_A);	/* Set Tx and Rx rates to 9600 */
	mode1 = SCN2681_MODE1_BPC_8 | SCN2681_MODE1_PM_NO | SCN2681_MODE1_RXINT_RXRDY | SCN2681_MODE1_RXRTS_ON;
	writeb(mode1, base + SCN2681_REG_MODE_A);							/* Mode 1: 8-bit, No Parity, RTS On */
	mode2 = SCN2681_MODE2_CHMODE_LLOOP | SCN2681_MODE2_TXRTS_OFF | SCN2681_MODE2_TXCTS_OFF | SCN2681_MODE2_SBL_1000;
	writeb(mode2, base + SCN2681_REG_MODE_A);							/* Mode 2: Normal Mode, Not CTS/RTS, 1 stop bit */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND_A);			/* Enable Transmitter / Receiver */

	// Configure Channel B
	writeb(((SCN2681_BRG1_1200 << 4) | SCN2681_BRG1_1200), base + SCN2681_REG_CLK_SELECT_B);	/* Set Tx and Rx rates to 1200/115200 */
	mode1 = SCN2681_MODE1_BPC_8 | SCN2681_MODE1_PM_NO | SCN2681_MODE1_RXINT_RXRDY | SCN2681_MODE1_RXRTS_ON;
	writeb(mode1, base + SCN2681_REG_MODE_B);							/* Mode 1: 8-bit, No Parity, RTS On */
	mode2 = SCN2681_MODE2_CHMODE_LLOOP | SCN2681_MODE2_TXRTS_OFF | SCN2681_MODE2_TXCTS_OFF | SCN2681_MODE2_SBL_1000;
	writeb(mode2, base + SCN2681_REG_MODE_B);							/* Mode 2: Normal Mode, Not CTS/RTS, 1 stop bit */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND_B);			/* Enable Transmitter / Receiver */

	// Write character to Tx holding registers
	writeb('G', base + SCN2681_REG_TX_A);
	writeb('G', base + SCN2681_REG_TX_B);

	// Poll for result
	while (loop > 0) {
		// Read ISR status
		tmp = readb(base + SCN2681_REG_IRQ_STATUS);
		// Check if there's already a result
		if (!isr) {
			// Check if one of the flags is set in the ISR
			if (tmp & (SCN2681_ISR_RX_RDY_A | SCN2681_ISR_RX_RDY_B)) isr = tmp;
		}
		// Make sure the Txs are empty before completing
		// Don't want garbage being transmitted to the receiver
		// when they're initialised in _scn2681_serial_init
		if ((tmp & (SCN2681_ISR_RX_RDY_A | SCN2681_ISR_RX_RDY_B)) == (SCN2681_ISR_RX_RDY_A | SCN2681_ISR_RX_RDY_B)) break;
		loop--;
	}

	// If Channel B set, the BRG Test mode is enable, so toggle it
	if (isr & SCN2681_ISR_RX_RDY_B) readb(base + SCN2681_REG_BRG_TEST);
}

static void _scn2681_serial_init(fdt_addr_t base)
{
	// Reset Channel A
	writeb(SCN2681_CMD_RST_TX, base + SCN2681_REG_COMMAND_A);					/* Reset Transmitter */
	writeb(SCN2681_CMD_RST_RX, base + SCN2681_REG_COMMAND_A);					/* Reset Receiver */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND_A);					/* Reset Mode Register Pointer */

	// Reset Channel B
	writeb(SCN2681_CMD_RST_TX, base + SCN2681_REG_COMMAND_B);					/* Reset Transmitter */
	writeb(SCN2681_CMD_RST_RX, base + SCN2681_REG_COMMAND_B);					/* Reset Receiver */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND_B);					/* Reset Mode Register Pointer */

	/* Baud Rate Set #2 */
	writeb(SCN2681_ACR_BRG_SELECT, base + SCN2681_REG_AUX_CTRL);

	// Configure Channel A
	writeb(((SCN2681_BRG1_9600 << 4) | SCN2681_BRG1_9600), base + SCN2681_REG_CLK_SELECT_A);	/* Set Tx and Rx rates to 9600 */
	unsigned char mode1 = SCN2681_MODE1_BPC_8 | SCN2681_MODE1_PM_NO | SCN2681_MODE1_RXRTS_ON;
	writeb(mode1, base + SCN2681_REG_MODE_A);							/* Mode 1: 8-bit, No Parity, RTS On */
	unsigned char mode2 = SCN2681_MODE2_CHMODE_NORMAL | SCN2681_MODE2_TXRTS_OFF | SCN2681_MODE2_TXCTS_OFF | SCN2681_MODE2_SBL_1000;
	writeb(mode2, base + SCN2681_REG_MODE_A);							/* Mode 2: Normal Mode, Not CTS/RTS, 1 stop bit */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND_A);			/* Enable Transmitter / Receiver */
}

static int scn2681_serial_probe(struct udevice *dev)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);
//	struct clk clk;
//	int ret;

//	ret = clk_get_by_index(dev, 0, &clk);
//	if (ret < 0)
//		return ret;

//	ret = clk_enable(&clk);
//	if (ret) {
//		dev_err(dev, "failed to enable clock\n");
//		return ret;
//	}

//	plat->clock_rate = clk_get_rate(&clk);
//	if (!plat->clock_rate) {
//		clk_disable(&clk);
//		return -EINVAL;
//	}

	// Reset BRG Test Mode
	_scn2681_serial_brgtest_reset(plat->base);
	// Initialise device
	_scn2681_serial_init(plat->base);

	return 0;
}

static const struct udevice_id scn2681_serial_id[] = {
	{ .compatible = "philips,scn2681-uart" },
	{}
};

static int scn2681_serial_of_to_plat(struct udevice *dev)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr(dev);
	if (plat->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->brg_testmode = false;

	return 0;
}

static const struct dm_serial_ops scn2681_serial_ops = {
	.putc = scn2681_serial_putc,
	.pending = scn2681_serial_pending,
	.getc = scn2681_serial_getc,
	.setbrg = scn2681_serial_setbrg,
	.setconfig = scn2681_serial_setconfig
};

U_BOOT_DRIVER(serial_scn2681) = {
	.name		= "serial_scn2681",
	.id		= UCLASS_SERIAL,
	.of_match	= of_match_ptr(scn2681_serial_id),
	.of_to_plat	= of_match_ptr(scn2681_serial_of_to_plat),
	.plat_auto	= sizeof(struct scn2681_serial_plat),
	.ops		= &scn2681_serial_ops,
	.probe		= scn2681_serial_probe,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};
