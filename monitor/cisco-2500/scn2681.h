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

/* SCN2681 DUART Mode Register 1 (R/W) */
SCN2681_REG_MODE1_BPC_5			= 0x0				/* Bits Per Character: 5 */
SCN2681_REG_MODE1_BPC_6			= 0x1				/* Bits Per Character: 6 */
SCN2681_REG_MODE1_BPC_7			= 0x2				/* Bits Per Character: 7 */
SCN2681_REG_MODE1_BPC_8			= 0x3				/* Bits Per Character: 8 */
SCN2681_REG_MODE1_PARITY		= 1<<2				/* Parity: 0 - Even / 1 - Odd */
SCN2681_REG_MODE1_WITH_PARITY		= 0x0<<3			/* Parity Mode: With Parity */
SCN2681_REG_MODE1_FORCE_PARITY		= 0x1<<3			/* Parity Mode: Force Parity */
SCN2681_REG_MODE1_NO_PARITY		= 0x2<<3			/* Parity Mode: No Parity */
SCN2681_REG_MODE1_MULTIDROP		= 0x3<<3			/* Multidrop Mode */
SCN2681_REG_MODE1_ERROR_MODE		= 1<<5				/* Error Mode: 0 - Character / 1 - Block */
SCN2681_REG_MODE1_RX_INT		= 1<<6				/* Rx Interrupt: 0 - RxRDY / 1 - FFULL */
SCN2681_REG_MODE1_RX_RTS		= 1<<7				/* Rx RTS Control: 0 - No / 1 - Yes */

/* SCN2681 DUART Mode Register 2 (R/W) */
SCN2681_REG_MODE2_STOP_075		= 0x3				/* Stop Bit Length: 0.75 */
SCN2681_REG_MODE2_STOP_100		= 0x7				/* Stop Bit Length: 1.00 */
SCN2681_REG_MODE2_STOP_175		= 0xb				/* Stop Bit Length: 1.75 */
SCN2681_REG_MODE2_STOP_200		= 0xf				/* Stop Bit Length: 2.00 */
SCN2681_REG_MODE2_TX_CTS		= 1<<4				/* Tx CTS: 0 - Disabled / 1 - Enabled */
SCN2681_REG_MODE2_TX_RTS		= 1<<5				/* Tx RTS: 0 - Disabled / 1 - Enabled */
SCN2681_REG_MODE2_MODE_NORMAL		= 0<<6				/* Channel Mode: Normal */
SCN2681_REG_MODE2_MODE_AUTO_ECHO	= 1<<6				/* Channel Mode: Auto-Echo */
SCN2681_REG_MODE2_MODE_LOCAL_LOOP	= 2<<6				/* Channel Mode: Local Loop */
SCN2681_REG_MODE2_MODE_REMOTE_LOOP	= 3<<6				/* Channel Mode: Remote Loop */

/* SCN2681 DUART Status Register (R) */
SCN2681_REG_STATUS_RX_RDY		= 1<<0				/* Rx Ready: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_FFULL		= 1<<1				/* FFULL: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_TX_RDY		= 1<<2				/* Tx Ready: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_TX_EMT		= 1<<3				/* Tx EMT: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_OVERRUN_ERROR	= 1<<4				/* Overrun Error: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_PARITY_ERROR		= 1<<5				/* Parity Error: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_FRAMING_ERROR	= 1<<6				/* Framing Error: 0 - No / 1 - Yes */
SCN2681_REG_STATUS_RECEIVED_BREAK	= 1<<7				/* Received Break: 0 - No / 1 - Yes */

/* SCN2681 DUART Command Register (W) */
SCN2681_REG_COMMAND_RX_ENABLE		= 1<<0				/* Enable Receiver */
SCN2681_REG_COMMAND_RX_DISABLE		= 1<<1				/* Disable Receiver */
SCN2681_REG_COMMAND_TX_ENABLE		= 1<<2				/* Enable Transmitter */
SCN2681_REG_COMMAND_TX_DISABLE		= 1<<3				/* Disable Transmitter */
SCN2681_REG_COMMAND_MISC_RESET_MR	= 1<<4				/* Reset MR Pointer */
SCN2681_REG_COMMAND_MISC_RESET_RX	= 2<<4				/* Reset Receiver */
SCN2681_REG_COMMAND_MISC_RESET_TX	= 3<<4				/* Reset Transmitter */
SCN2681_REG_COMMAND_MISC_RESET_ERR	= 4<<4				/* Reset Error Status */
SCN2681_REG_COMMAND_MISC_RESET_CHNG	= 5<<4				/* Reset Break Change Interrupt */
SCN2681_REG_COMMAND_MISC_BREAK_START	= 6<<4				/* Start Break */
SCN2681_REG_COMMAND_MISC_BREAK_STOP	= 7<<4				/* Stop Break */

/* SCN2681 DUART Auxiliary Control Register (W) */
SCN2681_REG_AUX_CONTROL_INT_DELTA_IP0	= 1<<0				/* IP0 Delta Interrupt: 0 - Off / 1 - On */
SCN2681_REG_AUX_CONTROL_INT_DELTA_IP1	= 1<<1				/* IP1 Delta Interrupt: 0 - Off / 1 - On */
SCN2681_REG_AUX_CONTROL_INT_DELTA_IP2	= 1<<2				/* IP2 Delta Interrupt: 0 - Off / 1 - On */
SCN2681_REG_AUX_CONTROL_INT_DELTA_IP3	= 1<<3				/* IP3 Delta Interrupt: 0 - Off / 1 - On */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_0	= 0<<4				/* Mode: Counter   Source: External (IP2) */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_1	= 1<<4				/* Mode: Counter   Source: TxCA (1x clock of Channel A transmitter) */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_2	= 2<<4				/* Mode: Counter   Source: TxCB (1x clock of Channel B transmitter) */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_3	= 3<<4				/* Mode: Counter   Source: Crystal or external clock (x1/CLK) divided by 16 */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_4	= 4<<4				/* Mode: Timer     Source: External (IP2) */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_5	= 5<<4				/* Mode: Timer     Source: External (IP2) divided by 16 */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_6	= 6<<4				/* Mode: Timer     Source: Crystal or external clock (x1/CLK) */
SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_7	= 7<<4				/* Mode: Timer     Source: Crystal or external clock (x1/CLK) divided by 16 */
SCN2681_REG_AUX_CONTROL_BRG_SET_SELECT	= 1<<7				/* Baud Rate Generator Set Select */
									/* 0 - Set 1: 50, 110, 134.5, 200, 300, 600, 1.05k, 1.2k, 2.4k, 4.8k, 7.2k, 9.6k, and 38.4k baud. */
									/* 1 - Set 2: 75, 110, 134.5, 150, 300, 600, 1.2k, 1.8k, 2.0k, 2.4k, 4.8k, 9.6k, and 19.2k baud. */

/* SCN2681 DUART Interrupt Status Register (R) / Interrupt Mask Register (W) */
SCN2681_REG_INTERRUPT_CHANNELA_TX_RDY	= 1<<0				/* Channel A Tx Ready */
SCN2681_REG_INTERRUPT_CHANNELA_RX_RDY	= 1<<1				/* Channel A Rx Ready Or FIFO Full */
SCN2681_REG_INTERRUPT_CHANNELA_BREAK	= 1<<2				/* Channel A Change in Break */
SCN2681_REG_INTERRUPT_COUNTER_READY	= 1<<3				/* Counter Ready */
SCN2681_REG_INTERRUPT_CHANNELB_TX_RDY	= 1<<4				/* Channel B Tx Ready */
SCN2681_REG_INTERRUPT_CHANNELB_RX_RDY	= 1<<5				/* Channel B Rx Ready Or FIFO Full */
SCN2681_REG_INTERRUPT_CHANNELB_BREAK	= 1<<6				/* Channel B Change in Break */
SCN2681_REG_INTERRUPT_INPUT_PORT_CHG	= 1<<7				/* Input Port Change Status */

/* SCN2681 DUART Output Port Configuration Register (R) */
SCN2681_REG_OUTPUT_PORT_CFG_2_OPR2	= 0x0				/* Output Port[2]: Output Port Register[2] */
SCN2681_REG_OUTPUT_PORT_CFG_2_TXCA_X16	= 0x1				/* Output Port[2]: Channel A transmitter clock x16 */
SCN2681_REG_OUTPUT_PORT_CFG_2_TXCA_X1	= 0x2				/* Output Port[2]: Channel A transmitter clock x1 */
SCN2681_REG_OUTPUT_PORT_CFG_2_RXCA_X1	= 0x3				/* Output Port[2]: Channel A receiver clock x1 */
SCN2681_REG_OUTPUT_PORT_CFG_3_OPR3	= 0x0<<2			/* Output Port[3]: Output Port Register[3] */
SCN2681_REG_OUTPUT_PORT_CFG_3_CT_OUT	= 0x1<<2			/* Output Port[3]: Counter/Timer output */
SCN2681_REG_OUTPUT_PORT_CFG_3_TXCB_X1	= 0x2<<2			/* Output Port[3]: Channel B transmitter clock x1 */
SCN2681_REG_OUTPUT_PORT_CFG_3_RXCB_X1	= 0x3<<2			/* Output Port[3]: Channel B receiver clock x1 */
SCN2681_REG_OUTPUT_PORT_CFG_4		= 0x1<<4			/* Output Port[4]: 0 - Output Port Register[4] / Channel A Rx Ready (FFULLA) */
SCN2681_REG_OUTPUT_PORT_CFG_5		= 0x1<<5			/* Output Port[5]: 0 - Output Port Register[5] / Channel B Rx Ready (FFULLB) */
SCN2681_REG_OUTPUT_PORT_CFG_6		= 0x1<<6			/* Output Port[6]: 0 - Output Port Register[6] / Channel A Tx Ready */
SCN2681_REG_OUTPUT_PORT_CFG_7		= 0x1<<7			/* Output Port[7]: 0 - Output Port Register[7] / Channel B Tx Ready */

