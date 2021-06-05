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

static void _scn2681_serial_setbrg(fdt_addr_t base,
				 u32 clock_rate,
				 int baudrate)
{
	unsigned char clk_select = 0, brg_select = 0;

	switch (baudrate) {
	case 150:
		clk_select = (SCN2681_BRG1_150 << 4) | SCN2681_BRG1_150;
		brg_select = 1;
		break;
	case 300:
		clk_select = (SCN2681_BRG1_300 << 4) | SCN2681_BRG1_300;
		brg_select = 1;
		break;
	case 600:
		clk_select = (SCN2681_BRG1_600 << 4) | SCN2681_BRG1_600;
		brg_select = 1;
		break;
	case 1200:
		clk_select = (SCN2681_BRG1_1200 << 4) | SCN2681_BRG1_1200;
		brg_select = 1;
		break;
	case 2400:
		clk_select = (SCN2681_BRG1_2400 << 4) | SCN2681_BRG1_2400;
		brg_select = 1;
		break;
	case 4800:
		clk_select = (SCN2681_BRG1_4800 << 4) | SCN2681_BRG1_4800;
		brg_select = 1;
		break;
	default:
	case 9600:
		clk_select = (SCN2681_BRG1_9600 << 4) | SCN2681_BRG1_9600;
		brg_select = 1;
		break;
	case 19200:
		clk_select = (SCN2681_BRG1_19200 << 4) | SCN2681_BRG1_19200;
		brg_select = 1;
		break;
	case 38400:
		clk_select = (SCN2681_BRG0_38400 << 4) | SCN2681_BRG0_38400;
		brg_select = 0;
		break;
	}

	/* Shutdown UART */
	writeb(SCN2681_CMD_DSB_TX, base + SCN2681_REG_COMMAND);						/* Disable Transmitter */
	writeb(SCN2681_CMD_DSB_RX, base + SCN2681_REG_COMMAND);						/* Disable Reciever */
	if (brg_select)
		writeb(0x80, base + SCN2681_REG_AUX_CTRL);						/* Baud Rate set 2 */
	writeb(clk_select, base + SCN2681_REG_CLK_SELECT);						/* Set Tx and Rx rates */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND);			/* Enable Transmitter / Receiver */
}

static int scn2681_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);

	_scn2681_serial_setbrg(plat->base, plat->clock_rate, baudrate);

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
	writeb(SCN2681_CMD_DSB_TX, base + SCN2681_REG_COMMAND);						/* Disable Transmitter */
	writeb(SCN2681_CMD_DSB_RX, base + SCN2681_REG_COMMAND);						/* Disable Reciever */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND);						/* Reset Mode Register Pointer */

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
	writeb(mode1, base + SCN2681_REG_MODE);								/* Mode 1: 8-bit, No Parity, RTS On */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND);			/* Enable Transmitter / Receiver */

	return 0;
}

static int scn2681_serial_getc(struct udevice *dev)
{
	struct scn2681_serial_plat *plat = dev_get_plat(dev);
	fdt_addr_t base = plat->base;

	unsigned char uart_status = readb(base + SCN2681_REG_STATUS);
	if ((uart_status & SCN2681_STATUS_RXRDY) == 0)
		return -EAGAIN;

	if (uart_status & (SCN2681_STATUS_ERR_OVER | SCN2681_STATUS_ERR_PARITY | SCN2681_STATUS_ERR_FRAME)) {
		writeb(SCN2681_CMD_RST_ERR, base + SCN2681_REG_COMMAND);
		return -EIO;
	}

	return readb(base + SCN2681_REG_RX);
}

static int _scn2681_serial_putc(fdt_addr_t base,
			      const char c)
{
	if ((readb(base + SCN2681_REG_STATUS) & SCN2681_STATUS_TXRDY) == 0)
		return -EAGAIN;

	writeb(c, base + SCN2681_REG_TX);

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
		return readb(base + SCN2681_REG_STATUS) & SCN2681_STATUS_RXRDY ? 1 : 0;
	else
		return readb(base + SCN2681_REG_STATUS) & SCN2681_STATUS_TXRDY ? 0 : 1;
}

static void _scn2681_serial_init(fdt_addr_t base)
{
	writeb(SCN2681_CMD_RST_TX, base + SCN2681_REG_COMMAND);						/* Reset Transmitter */
	writeb(SCN2681_CMD_RST_RX, base + SCN2681_REG_COMMAND);						/* Reset Reciever */
	writeb(SCN2681_CMD_RST_MR, base + SCN2681_REG_COMMAND);						/* Reset Mode Register Pointer */
	writeb(0x80, base + SCN2681_REG_AUX_CTRL);							/* Baud Rate Set #2 */
	writeb(((SCN2681_BRG1_9600 << 4) | SCN2681_BRG1_9600), base + SCN2681_REG_CLK_SELECT);		/* Set Tx and Rx rates to 9600 */
	unsigned char mode1 = SCN2681_MODE1_BPC_8 | SCN2681_MODE1_PM_NO | SCN2681_MODE1_RXRTS_ON;
	writeb(mode1, base + SCN2681_REG_MODE);								/* Mode 1: 8-bit, No Parity, RTS On */
	unsigned char mode2 = SCN2681_MODE2_CHMODE_NORMAL | SCN2681_MODE2_TXRTS_OFF | SCN2681_MODE2_TXCTS_OFF | SCN2681_MODE2_SBL_1000;
	writeb(mode2, base + SCN2681_REG_MODE);								/* Mode 2: Normal Mode, Not CTS/RTS, 1 stop bit */
	writeb((SCN2681_CMD_EN_RX | SCN2681_CMD_EN_TX), base + SCN2681_REG_COMMAND);			/* Enable Transmitter / Receiver */
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
