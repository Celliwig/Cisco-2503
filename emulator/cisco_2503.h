#ifndef C2503__HEADER
#define C2503__HEADER

// Cisco hardware defines
#define	C2503_CPU			M68K_CPU_TYPE_68EC030

// Boot ROM (PLCC32 FW1/FW2)
#define C2503_BOOTROM_ADDR1		0x00000000			// Start address of boot ROM
#define C2503_BOOTROM_ADDR2		0x01000000			// Alternate address of boot ROM
#define C2503_BOOTROM_SIZE		0x200000			// 2MB Boot ROM

// RAM
#define C2503_RAM_ADDR1			0x00000000			// Start address of RAM
#define C2503_RAM_SIZE			0x100				// 256 bytes of RAM

// NVRAM
#define	C2503_NVRAM_ADDR		0x02000000			// NVRAM address
#define	C2503_NVRAM_SIZE		0x20000				// NVRAM window size (128k, maybe actually 32k)

// I/O
#define C2503_IO_ADDR			0x02100000			// I/O start address
#define C2503_IO_SIZE			0x40000				// 256k window for I/O
#define C2503_IO_68302_REG_ADDR		0x02100000			// 68302 registers address
#define C2503_IO_68302_REG_SIZE		0x1000				// 68302 registers window size
#define C2503_IO_68302_RAM_ADDR		0x02101000			// 68302 RAM address
#define C2503_IO_68302_RAM_SIZE		0x1000				// 68302 RAM window size
#define C2503_IO_SYS_CONTROL1_ADDR	0x02110000			// System control register 1 address
#define C2503_IO_SYS_CONTROL1_SIZE	0x2				// System control register 1 size
#define C2503_IO_SYS_CONTROL2_ADDR	0x02110002			// System control register 1 address
#define C2503_IO_SYS_CONTROL2_SIZE	0x1				// System control register 1 size
#define C2503_IO_SYS_STATUS_ADDR	0x02110100			// System status register address
#define C2503_IO_SYS_STATUS_SIZE	0x1				// System status register size
#define C2503_IO_SYS_ID_COOKIE_ADDR	0x02110040			// System ID cookie address
#define C2503_IO_SYS_ID_COOKIE_SIZE	0x20				// System ID cookie size
#define C2503_IO_COUNTER_TIMER_ADDR	0x02120000			// Counter/Timer regiter address
#define C2503_IO_COUNTER_TIMER_SIZE	0x4				// Counter/Timer register size
#define C2503_IO_COUNTER_CONTROL_ADDR	0x02120040			// Counter/Timer control register address
#define C2503_IO_COUNTER_CONTROL_SIZE	0x1				// Counter/Timer control register size
#define C2503_IO_DUART_ADDR		0x02120100			// DUART address
#define C2503_IO_DUART_SIZE		0x40				// DUART size
#define C2503_IO_CHANNELA_LANCE_ADDR	0x02130000			// Channel A: LANCE address
#define C2503_IO_CHANNELA_LANCE_SIZE	0x4				// Channel A: LANCE size
#define C2503_IO_CHANNELB_LANCE_ADDR	0x02130040			// Channel B: LANCE/serial address
#define C2503_IO_CHANNELB_LANCE_SIZE	0x4				// Channel B: LANCE/serial size
#define C2503_IO_CHANNELB_SERIAL_ADDR	0x02130080			// Channel B: serial DTR address
#define C2503_IO_CHANNELB_SERIAL_SIZE	0x4				// Channel B: serial DTR size

/* IRQ connections */
#define IRQ_NMI_DEVICE			7
#define IRQ_INPUT_DEVICE		2
#define IRQ_OUTPUT_DEVICE		1

/* Time between characters sent to output device (seconds) */
#define OUTPUT_DEVICE_PERIOD		1

// NCurses definitions
#define EMU_WIN_CODE_COLS_MAX		75				// Maximum number of columns to allocate to the code window
#define EMU_WIN_REG_ROWS_MAX		7				// Maximum number of rows to allocate to the registers window
#define EMU_WIN_STATUS_ROWS_MAX		1				// Maximum number of rows to allocate to the status window

#define EMU_WIN_MEM_DISP_SPACE		1<<7				// Memory window displays (0 - Data space / 1 - Program space)
#define EMU_WIN_MEM_DISP_MASK		(0xff ^ EMU_WIN_MEM_DISP_SPACE)
#define EMU_WIN_MEM_DISP_A0		0				// Memory window display address from A0
#define EMU_WIN_MEM_DISP_A1		1				// Memory window display address from A1
#define EMU_WIN_MEM_DISP_A2		2				// Memory window display address from A2
#define EMU_WIN_MEM_DISP_A3		3				// Memory window display address from A3
#define EMU_WIN_MEM_DISP_A4		4				// Memory window display address from A4
#define EMU_WIN_MEM_DISP_A5		5				// Memory window display address from A5
#define EMU_WIN_MEM_DISP_A6		6				// Memory window display address from A6
#define EMU_WIN_MEM_DISP_A7		7				// Memory window display address from A7
#define EMU_WIN_MEM_DISP_PC		8				// Memory window display address from PC
#define EMU_WIN_MEM_DISP_SELECTED	127				// Memory window display selected address

// Disassembler
#define cpu_read_word_dasm		mem_pgrm_read_word
#define cpu_read_long_dasm		mem_pgrm_read_long

// Macros
//////////////////////////////////////////////////////////////////////////////////////////////

// Prototypes
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned int cpu_read_byte(unsigned int address);
unsigned int cpu_read_word(unsigned int address);
unsigned int cpu_read_long(unsigned int address);
void cpu_write_byte(unsigned int address, unsigned int value);
void cpu_write_word(unsigned int address, unsigned int value);
void cpu_write_long(unsigned int address, unsigned int value);
void cpu_pulse_reset(void);
void cpu_set_fc(unsigned int fc);
int  cpu_irq_ack(int level);
void cpu_instr_callback(int pc);

#endif /* C2503__HEADER */
