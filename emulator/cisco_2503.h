#ifndef C2503__HEADER
#define C2503__HEADER

// Cisco hardware defines
#define	C2503_CPU		M68K_CPU_TYPE_68EC030

// Boot ROM (PLCC32 FW1/FW2)
#define C2503_BOOTROM_SIZE	0x200000			// 2MB Boot ROM
#define C2503_BOOTROM_ADDR1	0x00000000			// Starting location of boot ROM
#define C2503_BOOTROM_ADDR2	0x01000000			// Alternate location of boot ROM

#define C2503_RAM_SIZE		0x100				// 256 bytes of RAM
#define C2503_RAM_ADDR1		0x00000000			// Starting location of RAM

/* Old Memory-mapped IO ports */
#define INPUT_ADDRESS		0x800000
#define OUTPUT_ADDRESS		0x400000

/* IRQ connections */
#define IRQ_NMI_DEVICE		7
#define IRQ_INPUT_DEVICE	2
#define IRQ_OUTPUT_DEVICE	1

/* Time between characters sent to output device (seconds) */
#define OUTPUT_DEVICE_PERIOD	1

// NCurses definitions
#define EMU_WIN_CODE_COLS_MAX	75				// Maximum number of columns to allocate to the code window
#define EMU_WIN_REG_ROWS_MAX	7				// Maximum number of rows to allocate to the registers window
#define EMU_WIN_STATUS_ROWS_MAX	1				// Maximum number of rows to allocate to the status window

// Disassembler
#define cpu_read_word_dasm	mem_pgrm_read_word
#define cpu_read_long_dasm	mem_pgrm_read_long

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
