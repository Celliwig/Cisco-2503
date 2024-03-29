/* IRQ connections */
#define IRQ_NMI_DEVICE			7
#define IRQ_INPUT_DEVICE		2
#define IRQ_OUTPUT_DEVICE		1

/* Time between characters sent to output device (seconds) */
#define OUTPUT_DEVICE_PERIOD		1

// System Register Intergrator
//////////////////////////////////////////////////////////////////////////////////////////////
#define SRI_BUFFER_SIZE			0x20

void disableSRI();
void enableSRI();
int getSRIFD();
void setSRIFD(int fd);
bool statusSRI();

void disableSRILogging();
void enableSRILogging();
bool statusSRILogging();

bool sriReadByte(unsigned int address, unsigned int *value);
bool sriReadWord(unsigned int address, unsigned int *value);
bool sriReadLong(unsigned int address, unsigned int *value);
bool sriWriteByte(unsigned int address, unsigned int value);
bool sriWriteWord(unsigned int address, unsigned int value);
bool sriWriteLong(unsigned int address, unsigned int value);

// Boot ROM
//////////////////////////////////////////////////////////////////////////////////////////////
#define C2503_BOOTROM_ADDR1		0x00000000			// Start address of boot ROM
#define C2503_BOOTROM_ADDR2		0x01000000			// Alternate address of boot ROM
#define C2503_BOOTROM_SIZE		0x200000			// 2MB Boot ROM
									// Flash ROMs are only 1MB (2x512) but left at 2MB
									// so can still run Cisco FW

bool mem_bootrom_init(FILE *fhandle);
bool mem_bootrom_split_init(FILE *fhandle1, FILE *fhandle2);
bool mem_bootrom_read_byte(unsigned address, unsigned int *value, bool real_read);
bool mem_bootrom_read_word(unsigned address, unsigned int *value);
bool mem_bootrom_read_long(unsigned address, unsigned int *value);
bool mem_bootrom_write_byte(unsigned int address, unsigned int value);
bool mem_bootrom_write_word(unsigned int address, unsigned int value);
bool mem_bootrom_write_long(unsigned int address, unsigned int value);

// Flash ROM
//////////////////////////////////////////////////////////////////////////////////////////////
#define C2503_FLASHROM_ADDR		0x03000000			// Address of main flash ROM
#define C2503_FLASHROM_SIZE		0x800000			// 8MB Flash ROM

bool mem_flashrom_init(FILE *fhandle);
bool mem_flashrom_read_byte(unsigned address, unsigned int *value);
bool mem_flashrom_read_word(unsigned address, unsigned int *value);
bool mem_flashrom_read_long(unsigned address, unsigned int *value);
bool mem_flashrom_write_byte(unsigned address, unsigned int value);
bool mem_flashrom_write_word(unsigned address, unsigned int value);
bool mem_flashrom_write_long(unsigned address, unsigned int value);

// NVRAM
//////////////////////////////////////////////////////////////////////////////////////////////
#define	C2503_NVRAM_ADDR		0x02000000			// NVRAM address
#define	C2503_NVRAM_SIZE		0x20000				// NVRAM window size (128k, maybe actually 32k)

void mem_nvram_init();
bool mem_nvram_read_byte(unsigned address, unsigned int *value);
bool mem_nvram_read_word(unsigned address, unsigned int *value);
bool mem_nvram_read_long(unsigned address, unsigned int *value);
bool mem_nvram_write_byte(unsigned address, unsigned int value);
bool mem_nvram_write_word(unsigned address, unsigned int value);
bool mem_nvram_write_long(unsigned address, unsigned int value);

// RAM
//////////////////////////////////////////////////////////////////////////////////////////////
#define	C2503_RAM_ADDR		0x00000000					// RAM address
#define	C2503_STD_RAM_SIZE	0x200000					// Preinstalled RAM size (2MB)
#define	C2503_DIMM_RAM_SIZE	0x200000					// DIMM RAM size (2MB)
//#define	C2503_DIMM_RAM_SIZE	0x0						// DIMM RAM size (0MB)
#define	C2503_RAM_SIZE		(C2503_STD_RAM_SIZE+C2503_DIMM_RAM_SIZE)	// RAM size (4MB)
#define	C2503_RAM_WIN_SIZE	0x1000000					// RAM window size (16MB)

bool mem_ram_read_byte(unsigned address, unsigned int *value);
bool mem_ram_read_word(unsigned address, unsigned int *value);
bool mem_ram_read_long(unsigned address, unsigned int *value);
bool mem_ram_write_byte(unsigned address, unsigned int value);
bool mem_ram_write_word(unsigned address, unsigned int value);
bool mem_ram_write_long(unsigned address, unsigned int value);

// 68302
//////////////////////////////////////////////////////////////////////////////////////////////
#define C2503_IO_68302_BASE_ADDR	0x02100000			// 68302 Base Address
#define C2503_IO_68302_BAR_ADDR		0xf0				// 68302 Base Address Register
#define C2503_IO_68302_SCR_ADDR		0xf4				// 68302 System Control Register
#define C2503_IO_68302_CKCR_ADDR	0xf8				// 68302 Clock Control Register

#define	C2503_IO_68302_RAM1_ADDR	0x0000				// 68302 System RAM
#define C2503_IO_68302_RAM1_SIZE	0x240				// 68302 RAM Window Size
#define	C2503_IO_68302_RAM2_ADDR	0x0400				// 68302 Parameter RAM
#define C2503_IO_68302_RAM2_SIZE	0x2c0				// 68302 RAM Window Size

#define C2503_IO_68302_REGS_ADDR	0x0800				// 68302 Internal Register Addresses
#define C2503_IO_68302_REGS_SIZE	0xb6				// 68302 Internal Register Window Size

void io_68302_core_init();
void io_68302_core_clock_tick();
bool io_68302_isIRQ();
bool io_68302_read_byte(unsigned address, unsigned int *value);
bool io_68302_read_word(unsigned address, unsigned int *value);
bool io_68302_read_long(unsigned address, unsigned int *value);
bool io_68302_write_byte(unsigned address, unsigned int value);
bool io_68302_write_word(unsigned address, unsigned int value);
bool io_68302_write_long(unsigned address, unsigned int value);

// System Registers
//////////////////////////////////////////////////////////////////////////////////////////////
// IRQ 3/4 appears associated

#define C2503_IO_SYS_CONTROL_BASE_ADDR		0x02110000			// System control registers base address
#define C2503_IO_SYS_CONTROL_WIN_SIZE		12				// System control registers window size

#define C2503_IO_SYS_CONTROL_HW_VERSION		0x0E				// Hardware version

#define C2503_IO_SYS_ID_COOKIE_ADDR		0x02110040			// Original System ID cookie address
#define C2503_IO_SYS_ID_COOKIE_SIZE		0x20				// Original System ID cookie size
#define C2503_IO_SYS_ID_COOKIE_ADDR2		0x02110060			// New System ID cookie interface
#define C2503_IO_SYS_ID_COOKIE_SIZE2		0x1				// New System ID cookie size
#define C2503_IO_SYS_STATUS_ADDR		0x02110100			// System status register address
#define C2503_IO_SYS_STATUS_SIZE		0x1				// System status register size

void io_system_core_init();
bool io_system_read_byte(unsigned address, unsigned int *value);
bool io_system_read_word(unsigned address, unsigned int *value);
bool io_system_read_long(unsigned address, unsigned int *value);
bool io_system_write_byte(unsigned address, unsigned int value);
bool io_system_write_word(unsigned address, unsigned int value);
bool io_system_write_long(unsigned address, unsigned int value);

// Counter / Timer
//////////////////////////////////////////////////////////////////////////////////////////////
#define	C2503_IO_COUNTER_REG1_ADDR	0x2120040
#define	C2503_IO_COUNTER_REG1_SIZE	0x1
#define	C2503_IO_COUNTER_REG2_ADDR	0x2120050
#define	C2503_IO_COUNTER_REG2_SIZE	0x2
#define	C2503_IO_COUNTER_REG3_ADDR	0x2120060
#define	C2503_IO_COUNTER_REG3_SIZE	0x2
#define	C2503_IO_COUNTER_REG4_ADDR	0x2120070
#define	C2503_IO_COUNTER_REG4_SIZE	0x2

void io_counter_core_init();
void io_counter_core_clock_tick();
bool io_system_cntl_reg_read(unsigned char address_offset, unsigned char *value);
bool io_system_cntl_reg_write(unsigned char address_offset, unsigned char value);
bool io_counter_read_byte(unsigned address, unsigned int *value);
bool io_counter_read_word(unsigned address, unsigned int *value);
bool io_counter_write_byte(unsigned address, unsigned int value);
bool io_counter_write_word(unsigned address, unsigned int value);

// Dual UART
//////////////////////////////////////////////////////////////////////////////////////////////
// Address & window size
#define C2503_IO_DUART_ADDR			0x02120100		// DUART address
#define C2503_IO_DUART_SIZE			0x40			// DUART size
#define C2503_IO_DUART_CORE_TICKS_PER_BYTE	0x7f			// Number of ticks to shift byte through TSR

// SCN2681 DUART registers (Read)
#define SCN2681_REG_RD_MODE_A			0x0			// Channel A: Mode Register 1/2
#define SCN2681_REG_RD_STATUS_A			0x1			// Channel A: Status Register
#define SCN2681_REG_RD_BRG_TEST			0x2			// BRG Test
#define SCN2681_REG_RD_RX_A			0x3			// Channel A: RX Register
#define SCN2681_REG_RD_INPUT_PORT_CHANGE	0x4			// Input Port Change Register
#define SCN2681_REG_RD_INTERRUPT_STATUS		0x5			// Interrupt Status Register
#define SCN2681_REG_RD_CTR_UPPER_VALUE		0x6			// Counter/Timer Upper Value Register
#define SCN2681_REG_RD_CTR_LOWER_VALUE		0x7			// Counter/Timer Lower Value Register
#define SCN2681_REG_RD_MODE_B			0x8			// Channel B: Mode Register 1/2
#define SCN2681_REG_RD_STATUS_B			0x9			// Channel B: Status Register
#define SCN2681_REG_RD_1X16X_TEST		0xa			// 1X/16X Test
#define SCN2681_REG_RD_RX_B			0xb			// Channel B: RX Register
#define SCN2681_REG_RD_RESERVED			0xc			// !!! Reserved !!!
#define SCN2681_REG_RD_INPUT_PORT		0xd			// Input Port Register
#define SCN2681_REG_RD_CTR_START_CMD		0xe			// Counter Start Command Register
#define SCN2681_REG_RD_CTR_STOP_CMD		0xf			// Counter Stop Command Register

// SCN2681 DUART registers (Write)
#define SCN2681_REG_WR_MODE_A			0x0			// Channel A: Mode Register 1/2
#define SCN2681_REG_WR_CLOCK_SELECT_A		0x1			// Channel A: Clock Select Register
#define SCN2681_REG_WR_COMMAND_A		0x2			// Channel A: Command Register
#define SCN2681_REG_WR_TX_A			0x3			// Channel A: TX Register
#define SCN2681_REG_WR_AUX_CONTROL		0x4			// Auxiliary Control Register
#define SCN2681_REG_WR_INTERRUPT_MASK		0x5			// Interrupt Mask Register
#define SCN2681_REG_WR_CTR_UPPER_VALUE_PRESET	0x6			// Counter/Timer Upper Value Preset Register
#define SCN2681_REG_WR_CTR_LOWER_VALUE_PRESET	0x7			// Counter/Timer Lower Value Preset Register
#define SCN2681_REG_WR_MODE_B			0x8			// Channel B: Mode Register 1/2
#define SCN2681_REG_WR_CLOCK_SELECT_B		0x9			// Channel B: Clock Select Register
#define SCN2681_REG_WR_COMMAND_B		0xa			// Channel B: Command Register
#define SCN2681_REG_WR_TX_B			0xb			// Channel B: TX Register
#define SCN2681_REG_WR_RESERVED			0xc			// !!! Reserved !!!
#define SCN2681_REG_WR_OUTPUT_PORT_CONF		0xd			// Output Port Configuration Register
#define SCN2681_REG_WR_OUTPUT_PORT_SET		0xe			// Output Port Set Register
#define SCN2681_REG_WR_OUTPUT_PORT_RESET	0xf			// Output Port Reset Register

// SCN2681 DUART Mode Register 1 (R/W)
#define SCN2681_REG_MODE1_BPC_5			0x0			// Bits Per Character: 5
#define SCN2681_REG_MODE1_BPC_6			0x1			// Bits Per Character: 6
#define SCN2681_REG_MODE1_BPC_7			0x2			// Bits Per Character: 7
#define SCN2681_REG_MODE1_BPC_8			0x3			// Bits Per Character: 8
#define SCN2681_REG_MODE1_PARITY		1<<2			// Parity: 0 - Even / 1 - Odd
#define SCN2681_REG_MODE1_WITH_PARITY		0x0<<3			// Parity Mode: With Parity
#define SCN2681_REG_MODE1_FORCE_PARITY		0x1<<3			// Parity Mode: Force Parity
#define SCN2681_REG_MODE1_NO_PARITY		0x2<<3			// Parity Mode: No Parity
#define SCN2681_REG_MODE1_MULTIDROP		0x3<<3			// Multidrop Mode
#define SCN2681_REG_MODE1_ERROR_MODE		1<<5			// Error Mode: 0 - Character / 1 - Block
#define SCN2681_REG_MODE1_RX_INT		1<<6			// Rx Interrupt: 0 - RxRDY / 1 - FFULL
#define SCN2681_REG_MODE1_RX_RTS		1<<7			// Rx RTS Control: 0 - No / 1 - Yes

// SCN2681 DUART Mode Register 2 (R/W)
#define SCN2681_REG_MODE2_STOP_075		0x3			// Stop Bit Length: 0.75
#define SCN2681_REG_MODE2_STOP_100		0x7			// Stop Bit Length: 1.00
#define SCN2681_REG_MODE2_STOP_175		0xb			// Stop Bit Length: 1.75
#define SCN2681_REG_MODE2_STOP_200		0xf			// Stop Bit Length: 2.00
#define SCN2681_REG_MODE2_TX_CTS		1<<4			// Tx CTS: 0 - Disabled / 1 - Enabled
#define SCN2681_REG_MODE2_TX_RTS		1<<5			// Tx RTS: 0 - Disabled / 1 - Enabled
#define SCN2681_REG_MODE2_MODE_NORMAL		0<<6			// Channel Mode: Normal
#define SCN2681_REG_MODE2_MODE_AUTO_ECHO	1<<6			// Channel Mode: Auto-Echo
#define SCN2681_REG_MODE2_MODE_LOCAL_LOOP	2<<6			// Channel Mode: Local Loop
#define SCN2681_REG_MODE2_MODE_REMOTE_LOOP	3<<6			// Channel Mode: Remote Loop

// SCN2681 DUART Status Register (R)
#define SCN2681_REG_STATUS_RX_RDY		1<<0			// Rx Ready: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_FFULL		1<<1			// FFULL: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_TX_RDY		1<<2			// Tx Ready: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_TX_EMT		1<<3			// Tx EMT: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_OVERRUN_ERROR	1<<4			// Overrun Error: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_PARITY_ERROR		1<<5			// Parity Error: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_FRAMING_ERROR	1<<6			// Framing Error: 0 - No / 1 - Yes
#define SCN2681_REG_STATUS_RECEIVED_BREAK	1<<7			// Received Break: 0 - No / 1 - Yes

// SCN2681 DUART Clock Select Register (W)

// SCN2681 DUART BRG Test Register (R)

// SCN2681 DUART Command Register (W)
#define SCN2681_REG_COMMAND_RX_ENABLE		1<<0			// Enable Receiver
#define SCN2681_REG_COMMAND_RX_DISABLE		1<<1			// Disable Receiver
#define SCN2681_REG_COMMAND_TX_ENABLE		1<<2			// Enable Transmitter
#define SCN2681_REG_COMMAND_TX_DISABLE		1<<3			// Disable Transmitter
#define SCN2681_REG_COMMAND_MISC_RESET_MR	1<<4			// Reset MR Pointer
#define SCN2681_REG_COMMAND_MISC_RESET_RX	2<<4			// Reset Receiver
#define SCN2681_REG_COMMAND_MISC_RESET_TX	3<<4			// Reset Transmitter
#define SCN2681_REG_COMMAND_MISC_RESET_ERR	4<<4			// Reset Error Status
#define SCN2681_REG_COMMAND_MISC_RESET_CHNG	5<<4			// Reset Break Change Interrupt
#define SCN2681_REG_COMMAND_MISC_BREAK_START	6<<4			// Start Break
#define SCN2681_REG_COMMAND_MISC_BREAK_STOP	7<<4			// Stop Break

#define SCN2681_REG_RX_FIFO_SIZE		3			// Number of character slots in Rx FIFO
// SCN2681 DUART RX FIFO Status Bits
#define SCN2681_REG_RX_FIFO_PARITY_ERR		1<<8			// Parity error associated with this byte
#define SCN2681_REG_RX_FIFO_FRAMING_ERR		1<<9			// Framing error associated with this byte
#define SCN2681_REG_RX_FIFO_RECEIVED_BRK	1<<10			// Received break associated with this byte
#define SCN2681_REG_RX_FIFO_VALID_BYTE		1<<15			// This is a valid byte

// SCN2681 DUART Auxiliary Control Register (W)
#define SCN2681_REG_AUX_CONTROL_INT_DELTA_IP0	1<<0			// IP0 Delta Interrupt: 0 - Off / 1 - On
#define SCN2681_REG_AUX_CONTROL_INT_DELTA_IP1	1<<1			// IP1 Delta Interrupt: 0 - Off / 1 - On
#define SCN2681_REG_AUX_CONTROL_INT_DELTA_IP2	1<<2			// IP2 Delta Interrupt: 0 - Off / 1 - On
#define SCN2681_REG_AUX_CONTROL_INT_DELTA_IP3	1<<3			// IP3 Delta Interrupt: 0 - Off / 1 - On
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_0	0<<4			// Mode: Counter   Source: External (IP2)
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_1	1<<4			// Mode: Counter   Source: TxCA (1x clock of Channel A transmitter)
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_2	2<<4			// Mode: Counter   Source: TxCB (1x clock of Channel B transmitter)
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_3	3<<4			// Mode: Counter   Source: Crystal or external clock (x1/CLK) divided by 16
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_4	4<<4			// Mode: Timer     Source: External (IP2)
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_5	5<<4			// Mode: Timer     Source: External (IP2) divided by 16
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_6	6<<4			// Mode: Timer     Source: Crystal or external clock (x1/CLK)
#define SCN2681_REG_AUX_CONTROL_CT_MODE_SRC_7	7<<4			// Mode: Timer     Source: Crystal or external clock (x1/CLK) divided by 16
#define SCN2681_REG_AUX_CONTROL_BRG_SET_SELECT	1<<7			// Baud Rate Generator Set Select
									// 0 - Set 1: 50, 110, 134.5, 200, 300, 600, 1.05k, 1.2k, 2.4k, 4.8k, 7.2k, 9.6k, and 38.4k baud.
									// 1 - Set 2: 75, 110, 134.5, 150, 300, 600, 1.2k, 1.8k, 2.0k, 2.4k, 4.8k, 9.6k, and 19.2k baud.

// SCN2681 DUART Interrupt Status Register (R) / Interrupt Mask Register (W)
#define SCN2681_REG_INTERRUPT_CHANNELA_TX_RDY	1<<0			// Channel A Tx Ready
#define SCN2681_REG_INTERRUPT_CHANNELA_RX_RDY	1<<1			// Channel A Rx Ready Or FIFO Full
#define SCN2681_REG_INTERRUPT_CHANNELA_BREAK	1<<2			// Channel A Change in Break
#define SCN2681_REG_INTERRUPT_COUNTER_READY	1<<3			// Counter Ready
#define SCN2681_REG_INTERRUPT_CHANNELB_TX_RDY	1<<4			// Channel B Tx Ready
#define SCN2681_REG_INTERRUPT_CHANNELB_RX_RDY	1<<5			// Channel B Rx Ready Or FIFO Full
#define SCN2681_REG_INTERRUPT_CHANNELB_BREAK	1<<6			// Channel B Change in Break
#define SCN2681_REG_INTERRUPT_INPUT_PORT_CHG	1<<7			// Input Port Change Status

// SCN2681 DUART Output Port Configuration Register (R)
#define SCN2681_REG_OUTPUT_PORT_CFG_2_OPR2	0x0			// Output Port[2]: Output Port Register[2]
#define SCN2681_REG_OUTPUT_PORT_CFG_2_TXCA_X16	0x1			// Output Port[2]: Channel A transmitter clock x16
#define SCN2681_REG_OUTPUT_PORT_CFG_2_TXCA_X1	0x2			// Output Port[2]: Channel A transmitter clock x1
#define SCN2681_REG_OUTPUT_PORT_CFG_2_RXCA_X1	0x3			// Output Port[2]: Channel A receiver clock x1
#define SCN2681_REG_OUTPUT_PORT_CFG_3_OPR3	0x0<<2			// Output Port[3]: Output Port Register[3]
#define SCN2681_REG_OUTPUT_PORT_CFG_3_CT_OUT	0x1<<2			// Output Port[3]: Counter/Timer output
#define SCN2681_REG_OUTPUT_PORT_CFG_3_TXCB_X1	0x2<<2			// Output Port[3]: Channel B transmitter clock x1
#define SCN2681_REG_OUTPUT_PORT_CFG_3_RXCB_X1	0x3<<2			// Output Port[3]: Channel B receiver clock x1
#define SCN2681_REG_OUTPUT_PORT_CFG_4		0x1<<4			// Output Port[4]: 0 - Output Port Register[4] / Channel A Rx Ready (FFULLA)
#define SCN2681_REG_OUTPUT_PORT_CFG_5		0x1<<5			// Output Port[5]: 0 - Output Port Register[5] / Channel B Rx Ready (FFULLB)
#define SCN2681_REG_OUTPUT_PORT_CFG_6		0x1<<6			// Output Port[6]: 0 - Output Port Register[6] / Channel A Tx Ready
#define SCN2681_REG_OUTPUT_PORT_CFG_7		0x1<<7			// Output Port[7]: 0 - Output Port Register[7] / Channel B Tx Ready

// SCN2681 DUART Register Names
enum scn2681_core_reg {
// Channel A
	ChannelA_Mode1,
	ChannelA_Mode2,
	ChannelA_Status,
	ChannelA_Clock_Select,
	ChannelA_Command,
	ChannelA_Rx_Fifo0,
	ChannelA_Rx_Fifo0_Status,
	ChannelA_Rx_Fifo1,
	ChannelA_Rx_Fifo1_Status,
	ChannelA_Rx_Fifo2,
	ChannelA_Rx_Fifo2_Status,
	ChannelA_Rx_Read_Index,
	ChannelA_Rx_Write_Index,
	ChannelA_Rx_Shift,
	ChannelA_Rx_Shift_Empty,
	ChannelA_Tx_Holding,
	ChannelA_Tx_Holding_Empty,
	ChannelA_Tx_Shift,
	ChannelA_Tx_Shift_Empty,
// Channel B
	ChannelB_Mode1,
	ChannelB_Mode2,
	ChannelB_Status,
	ChannelB_Clock_Select,
	ChannelB_Command,
	ChannelB_Rx_Fifo0,
	ChannelB_Rx_Fifo0_Status,
	ChannelB_Rx_Fifo1,
	ChannelB_Rx_Fifo1_Status,
	ChannelB_Rx_Fifo2,
	ChannelB_Rx_Fifo2_Status,
	ChannelB_Rx_Read_Index,
	ChannelB_Rx_Write_Index,
	ChannelB_Rx_Shift,
	ChannelB_Rx_Shift_Empty,
	ChannelB_Tx_Holding,
	ChannelB_Tx_Holding_Empty,
	ChannelB_Tx_Shift,
	ChannelB_Tx_Shift_Empty,
// Interrupts
	Interrupt_Mask,
	Interrupt_Status,
// Misc
	Auxiliary_Control,
// Counter / Timer
	CounterTimer_Upper,
	CounterTimer_Lower,
	CounterTimer_Upper_Preset,
	CounterTimer_Lower_Preset,
	CounterTimer_Start_Command,
	CounterTimer_Stop_Command,
// Input Port
	Input_Port,
	Input_Port_Change,
// Output Port
	Output_Port_Configuration,
	Output_Port_Set,
	Output_Port_Reset,
};

// Prototypes
void io_duart_core_channelA_serial_device(int device);
void io_duart_core_channelB_serial_device(int device);
unsigned char io_duart_core_get_reg(enum scn2681_core_reg regname);
void io_duart_core_init();
void io_duart_core_clock_tick();
bool io_duart_read_byte(unsigned address, unsigned int *value, bool real_read);
bool io_duart_write_byte(unsigned address, unsigned int value);

// Channel A: LANCE
//////////////////////////////////////////////////////////////////////////////////////////////
#define C2503_IO_CHANNELA_LANCE_RDP_ADDR	0x02130000		// Channel A: LANCE (Register Data Port)
#define C2503_IO_CHANNELA_LANCE_RAP_ADDR	0x02130002		// Channel A: LANCE (Register Address Port)

void io_channela_core_init();
void io_channela_core_clock_tick();
bool io_channela_isIRQ();
bool io_channela_read_word(unsigned address, unsigned int *value);
bool io_channela_write_word(unsigned address, unsigned int value);

// Channel B: Serial (HD64570)
//////////////////////////////////////////////////////////////////////////////////////////////
#define C2503_IO_CHANNELB_SERIAL_ADDR	0x02132000			// Channel B: Serial Address
#define C2503_IO_CHANNELB_SERIAL_SIZE	0xff				// Channel B: Serial Window Size

bool io_channelb_isIRQ();
bool io_channelb_read_byte(unsigned address, unsigned int *value);
bool io_channelb_read_word(unsigned address, unsigned int *value);
bool io_channelb_read_long(unsigned address, unsigned int *value);
bool io_channelb_write_byte(unsigned address, unsigned int value);
bool io_channelb_write_word(unsigned address, unsigned int value);
bool io_channelb_write_long(unsigned address, unsigned int value);

// Unknown1
//////////////////////////////////////////////////////////////////////////////////////////////
// IRQ 5?

#define C2503_IO_UNKNOWN1_ADDR		0x02132100
#define C2503_IO_UNKNOWN1_SIZE		0x8

bool io_unknown1_read_byte(unsigned address, unsigned int *value);
bool io_unknown1_read_word(unsigned address, unsigned int *value);
bool io_unknown1_read_long(unsigned address, unsigned int *value);
bool io_unknown1_write_byte(unsigned address, unsigned int value);
bool io_unknown1_write_word(unsigned address, unsigned int value);
bool io_unknown1_write_long(unsigned address, unsigned int value);
