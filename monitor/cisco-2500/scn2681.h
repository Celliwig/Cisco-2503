# SCN2681 Duart Register Addresses
##############################################################################

SCN2681_DUART_ADDR			= 0x02120100		/* Base Addr of DUART */

/* SCN2681 DUART registers (Read) */
SCN2681_ADDR_RD_MODE_A			= 0x0 + SCN2681_DUART_ADDR	/* Channel A: Mode Register 1/2 */
SCN2681_ADDR_RD_STATUS_A		= 0x1 + SCN2681_DUART_ADDR	/* Channel A: Status Register */
SCN2681_ADDR_RD_BRG_TEST		= 0x2 + SCN2681_DUART_ADDR	/* BRG Test */
SCN2681_ADDR_RD_RX_A			= 0x3 + SCN2681_DUART_ADDR	/* Channel A: RX Register */
SCN2681_ADDR_RD_INPUT_PORT_CHANGE	= 0x4 + SCN2681_DUART_ADDR	/* Input Port Change Register */
SCN2681_ADDR_RD_INTERRUPT_STATUS	= 0x5 + SCN2681_DUART_ADDR	/* Interrupt Status Register */
SCN2681_ADDR_RD_CTR_UPPER_VALUE		= 0x6 + SCN2681_DUART_ADDR	/* Counter/Timer Upper Value Register */
SCN2681_ADDR_RD_CTR_LOWER_VALUE		= 0x7 + SCN2681_DUART_ADDR	/* Counter/Timer Lower Value Register */
SCN2681_ADDR_RD_MODE_B			= 0x8 + SCN2681_DUART_ADDR	/* Channel B: Mode Register 1/2 */
SCN2681_ADDR_RD_STATUS_B		= 0x9 + SCN2681_DUART_ADDR	/* Channel B: Status Register */
SCN2681_ADDR_RD_1X16X_TEST		= 0xa + SCN2681_DUART_ADDR	/* 1X/16X Test */
SCN2681_ADDR_RD_RX_B			= 0xb + SCN2681_DUART_ADDR	/* Channel B: RX Register */
SCN2681_ADDR_RD_RESERVED		= 0xc + SCN2681_DUART_ADDR	/* !!! Reserved !!! */
SCN2681_ADDR_RD_INPUT_PORT		= 0xd + SCN2681_DUART_ADDR	/* Input Port Register */
SCN2681_ADDR_RD_CTR_START_CMD		= 0xe + SCN2681_DUART_ADDR	/* Counter Start Command Register */
SCN2681_ADDR_RD_CTR_STOP_CMD		= 0xf + SCN2681_DUART_ADDR	/* Counter Stop Command Register */

/* SCN2681 DUART registers (Write) */
SCN2681_ADDR_WR_MODE_A			= 0x0 + SCN2681_DUART_ADDR	/* Channel A: Mode Register 1/2 */
SCN2681_ADDR_WR_CLOCK_SELECT_A		= 0x1 + SCN2681_DUART_ADDR	/* Channel A: Clock Select Register */
SCN2681_ADDR_WR_COMMAND_A		= 0x2 + SCN2681_DUART_ADDR	/* Channel A: Command Register */
SCN2681_ADDR_WR_TX_A			= 0x3 + SCN2681_DUART_ADDR	/* Channel A: TX Register */
SCN2681_ADDR_WR_AUX_CONTROL		= 0x4 + SCN2681_DUART_ADDR	/* Auxiliary Control Register */
SCN2681_ADDR_WR_INTERRUPT_MASK		= 0x5 + SCN2681_DUART_ADDR	/* Interrupt Mask Register */
SCN2681_ADDR_WR_CTR_UPPER_VALUE_PRESET	= 0x6 + SCN2681_DUART_ADDR	/* Counter/Timer Upper Value Preset Register */
SCN2681_ADDR_WR_CTR_LOWER_VALUE_PRESET	= 0x7 + SCN2681_DUART_ADDR	/* Counter/Timer Lower Value Preset Register */
SCN2681_ADDR_WR_MODE_B			= 0x8 + SCN2681_DUART_ADDR	/* Channel B: Mode Register 1/2 */
SCN2681_ADDR_WR_CLOCK_SELECT_B		= 0x9 + SCN2681_DUART_ADDR	/* Channel B: Clock Select Register */
SCN2681_ADDR_WR_COMMAND_B		= 0xa + SCN2681_DUART_ADDR	/* Channel B: Command Register */
SCN2681_ADDR_WR_TX_B			= 0xb + SCN2681_DUART_ADDR	/* Channel B: TX Register */
SCN2681_ADDR_WR_RESERVED		= 0xc + SCN2681_DUART_ADDR	/* !!! Reserved !!! */
SCN2681_ADDR_WR_OUTPUT_PORT_CONF	= 0xd + SCN2681_DUART_ADDR	/* Output Port Configuration Register */
SCN2681_ADDR_WR_OUTPUT_PORT_SET		= 0xe + SCN2681_DUART_ADDR	/* Output Port Set Register */
SCN2681_ADDR_WR_OUTPUT_PORT_RESET	= 0xf + SCN2681_DUART_ADDR	/* Output Port Reset Register */
