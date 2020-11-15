#ifndef C2503__HEADER
#define C2503__HEADER

// Cisco hardware defines
#define	C2503_CPU			M68K_CPU_TYPE_68EC030

// NCurses definitions
#define EMU_WIN_CODE_COLS_MAX		75				// Maximum number of columns to allocate to the code window
#define EMU_WIN_DUART_ROWS_MAX		30				// Maximum number of rows to allocate to the DUART dialog
#define EMU_WIN_DUART_COLS_MAX		75				// Maximum number of columns to allocate to the DUART dialog
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
#define cpu_read_word_dasm		cpu_read_word
#define cpu_read_long_dasm		cpu_read_long

// Macros
//////////////////////////////////////////////////////////////////////////////////////////////
/* Read/write macros */
#define READ_BYTE(BASE, ADDR) (BASE)[ADDR]
#define READ_WORD(BASE, ADDR) (((BASE)[ADDR]<<8) |			\
							  (BASE)[(ADDR)+1])
#define READ_LONG(BASE, ADDR) (((BASE)[ADDR]<<24) |			\
							  ((BASE)[(ADDR)+1]<<16) |		\
							  ((BASE)[(ADDR)+2]<<8) |		\
							  (BASE)[(ADDR)+3])

#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[ADDR] = (VAL)&0xff
#define WRITE_WORD(BASE, ADDR, VAL) (BASE)[ADDR] = ((VAL)>>8) & 0xff;		\
									(BASE)[(ADDR)+1] = (VAL)&0xff
#define WRITE_LONG(BASE, ADDR, VAL) (BASE)[ADDR] = ((VAL)>>24) & 0xff;		\
									(BASE)[(ADDR)+1] = ((VAL)>>16)&0xff;	\
									(BASE)[(ADDR)+2] = ((VAL)>>8)&0xff;		\
									(BASE)[(ADDR)+3] = (VAL)&0xff

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
