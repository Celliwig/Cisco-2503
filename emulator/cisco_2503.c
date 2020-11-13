#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "cisco_2503.h"
#include "m68k.h"
//#include "osd.h"
#include <ncurses.h>

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


/* Prototypes */
void exit_error(char* fmt, ...);

unsigned int cpu_read_byte(unsigned int address);
unsigned int cpu_read_word(unsigned int address);
unsigned int cpu_read_long(unsigned int address);
void cpu_write_byte(unsigned int address, unsigned int value);
void cpu_write_word(unsigned int address, unsigned int value);
void cpu_write_long(unsigned int address, unsigned int value);
void cpu_pulse_reset(void);
void cpu_set_fc(unsigned int fc);
int cpu_irq_ack(int level);

void nmi_device_reset(void);
void nmi_device_update(void);
int nmi_device_ack(void);

void input_device_reset(void);
void input_device_update(void);
int input_device_ack(void);
unsigned int input_device_read(void);
void input_device_write(unsigned int value);

void output_device_reset(void);
void output_device_update(void);
int output_device_ack(void);
unsigned int output_device_read(void);
void output_device_write(unsigned int value);

void int_controller_set(unsigned int value);
void int_controller_clear(unsigned int value);

void get_user_input(void);

void emu_win_destroy();


/* Data */
unsigned int g_quit = 0;				// 1 if we want to quit
unsigned int g_nmi = 0;					// 1 if nmi pending

int          g_input_device_value = -1;			// Current value in input device

unsigned int g_output_device_ready = 0;			// 1 if output device is ready
time_t       g_output_device_last_output;		// Time of last char output

unsigned int g_int_controller_pending = 0;		// list of pending interrupts
unsigned int g_int_controller_highest_int = 0;		// Highest pending interrupt

unsigned char g_bootrom[C2503_BOOTROM_SIZE + 1];	// Boot ROM
unsigned char g_ram[C2503_RAM_SIZE + 1];		// RAM
unsigned int  g_fc;					// Current function code from CPU

// NCurses interface
unsigned int	emu_mem_dump_start = 0x00000000;	// Address to start memory dump from
unsigned char	emu_mem_dump_type = 0;			// Address space to dump (0 = Data / 1 = Program)
char	str_tmp_buf[512];				// Temporary string buffer
WINDOW	*emu_win_dialog = NULL, *emu_win_code = NULL, *emu_win_mem = NULL, *emu_win_reg = NULL;
unsigned short int xterm_cols, xterm_rows;
unsigned short int emu_win_dialog_cols, emu_win_dialog_rows, emu_win_dialog_y, emu_win_dialog_x;
unsigned short int emu_win_code_cols, emu_win_code_rows, emu_win_code_y, emu_win_code_x;
unsigned short int emu_win_mem_cols, emu_win_mem_rows, emu_win_mem_y, emu_win_mem_x;
unsigned short int emu_win_reg_cols, emu_win_reg_rows, emu_win_reg_y, emu_win_reg_x;

/* Exit with an error message.  Use printf syntax. */
void exit_error(char* fmt, ...)
{
	static int guard_val = 0;
	char buff[100];
	unsigned int pc;
	va_list args;

	if (guard_val)
		return;
	else
		guard_val = 1;

	// Destroy ncurses
	emu_win_destroy();
	endwin();

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
	pc = m68k_get_reg(NULL, M68K_REG_PPC);
	m68k_disassemble(buff, pc, C2503_CPU);
	fprintf(stderr, "At %04x: %s\n", pc, buff);

	exit(EXIT_FAILURE);
}


/* Called when the CPU pulses the RESET line */
void cpu_pulse_reset(void)
{
	nmi_device_reset();
	output_device_reset();
	input_device_reset();
}

/* Called when the CPU changes the function code pins */
void cpu_set_fc(unsigned int fc)
{
	g_fc = fc;
}

/* Called when the CPU acknowledges an interrupt */
int cpu_irq_ack(int level)
{
	switch(level)
	{
		case IRQ_NMI_DEVICE:
			return nmi_device_ack();
		case IRQ_INPUT_DEVICE:
			return input_device_ack();
		case IRQ_OUTPUT_DEVICE:
			return output_device_ack();
	}
	return M68K_INT_ACK_SPURIOUS;
}




/* Implementation for the NMI device */
void nmi_device_reset(void)
{
	g_nmi = 0;
}

void nmi_device_update(void)
{
	if(g_nmi)
	{
		g_nmi = 0;
		int_controller_set(IRQ_NMI_DEVICE);
	}
}

int nmi_device_ack(void)
{
	printf("\nNMI\n");fflush(stdout);
	int_controller_clear(IRQ_NMI_DEVICE);
	return M68K_INT_ACK_AUTOVECTOR;
}


/* Implementation for the input device */
void input_device_reset(void)
{
	g_input_device_value = -1;
	int_controller_clear(IRQ_INPUT_DEVICE);
}

void input_device_update(void)
{
	if(g_input_device_value >= 0)
		int_controller_set(IRQ_INPUT_DEVICE);
}

int input_device_ack(void)
{
	return M68K_INT_ACK_AUTOVECTOR;
}

unsigned int input_device_read(void)
{
	int value = g_input_device_value > 0 ? g_input_device_value : 0;
	int_controller_clear(IRQ_INPUT_DEVICE);
	g_input_device_value = -1;
	return value;
}

void input_device_write(unsigned int value)
{
	(void)value;
}


/* Implementation for the output device */
void output_device_reset(void)
{
	g_output_device_last_output = time(NULL);
	g_output_device_ready = 0;
	int_controller_clear(IRQ_OUTPUT_DEVICE);
}

void output_device_update(void)
{
	if(!g_output_device_ready)
	{
		if((time(NULL) - g_output_device_last_output) >= OUTPUT_DEVICE_PERIOD)
		{
			g_output_device_ready = 1;
			int_controller_set(IRQ_OUTPUT_DEVICE);
		}
	}
}

int output_device_ack(void)
{
	return M68K_INT_ACK_AUTOVECTOR;
}

unsigned int output_device_read(void)
{
	int_controller_clear(IRQ_OUTPUT_DEVICE);
	return 0;
}

void output_device_write(unsigned int value)
{
	char ch;
	if(g_output_device_ready)
	{
		ch = value & 0xff;
		printf("%c", ch);
		g_output_device_last_output = time(NULL);
		g_output_device_ready = 0;
		int_controller_clear(IRQ_OUTPUT_DEVICE);
	}
}


/* Implementation for the interrupt controller */
void int_controller_set(unsigned int value)
{
	unsigned int old_pending = g_int_controller_pending;

	g_int_controller_pending |= (1<<value);

	if(old_pending != g_int_controller_pending && value > g_int_controller_highest_int)
	{
		g_int_controller_highest_int = value;
		m68k_set_irq(g_int_controller_highest_int);
	}
}

void int_controller_clear(unsigned int value)
{
	g_int_controller_pending &= ~(1<<value);

	for(g_int_controller_highest_int = 7;g_int_controller_highest_int > 0;g_int_controller_highest_int--)
		if(g_int_controller_pending & (1<<g_int_controller_highest_int))
			break;

	m68k_set_irq(g_int_controller_highest_int);
}


/* Parse user input and update any devices that need user input */
void get_user_input(void)
{
	static int last_ch = -1;
//	int ch = osd_get_char();
	int ch = 0;

	if(ch >= 0)
	{
		switch(ch)
		{
			case 0x1b:
				g_quit = 1;
				break;
			case '~':
				if(last_ch != ch)
					g_nmi = 1;
				break;
			default:
				g_input_device_value = ch;
		}
	}
	last_ch = ch;
}

// Memory
//////////////////////////////////////////////////////////////////////////////////////////////
// Program
////////////////////////////////////////////////////////
unsigned int mem_pgrm_read_byte(unsigned int address, bool exit_on_error) {
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE))) {
		return READ_BYTE(g_bootrom, address - C2503_BOOTROM_ADDR1);
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		return READ_BYTE(g_bootrom, address - C2503_BOOTROM_ADDR2);
	}
	if (exit_on_error) {
		exit_error("Disassembler attempted to read word from ROM address %08x", address);
	}
	return -1;
}

unsigned int mem_pgrm_read_word(unsigned int address) {
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE))) {
		return READ_WORD(g_bootrom, address - C2503_BOOTROM_ADDR1);
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		return READ_WORD(g_bootrom, address - C2503_BOOTROM_ADDR2);
	}
	exit_error("Disassembler attempted to read word from ROM address %08x", address);
}

unsigned int mem_pgrm_read_long(unsigned int address) {
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE))) {
		return READ_LONG(g_bootrom, address - C2503_BOOTROM_ADDR1);
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		return READ_LONG(g_bootrom, address - C2503_BOOTROM_ADDR2);
	}
	exit_error("Dasm attempted to read long from ROM address %08x", address);
}

// Data
////////////////////////////////////////////////////////
unsigned int mem_data_read_byte(unsigned int address, bool exit_on_error) {
	if ((address >= C2503_RAM_ADDR1) && (address < (C2503_RAM_ADDR1 + C2503_RAM_SIZE))) {
		return READ_BYTE(g_ram, address);
	}
	if (exit_on_error) {
		exit_error("Attempted to read byte from RAM address %08x", address);
	}
	return -1;
}

unsigned int mem_data_read_word(unsigned int address) {
	if ((address >= C2503_RAM_ADDR1) && (address < (C2503_RAM_ADDR1 + C2503_RAM_SIZE))) {
		return READ_WORD(g_ram, address);
	}
	exit_error("Disassembler attempted to read word from RAM address %08x", address);
}

unsigned int mem_data_read_long(unsigned int address) {
	if ((address >= C2503_RAM_ADDR1) && (address < (C2503_RAM_ADDR1 + C2503_RAM_SIZE))) {
		return READ_LONG(g_ram, address);
	}
	exit_error("Dasm attempted to read long from RAM address %08x", address);
}

// Emulator
////////////////////////////////////////////////////////
// Read
////////////////////////////
unsigned int cpu_read_byte(unsigned int address) {
	// Program
	if(g_fc & 2) {
		return mem_pgrm_read_byte(address, true);
	}

	/* Otherwise it's data space */
	switch(address) {
		case INPUT_ADDRESS:
			return input_device_read();
		case OUTPUT_ADDRESS:
			return output_device_read();
		default:
			break;
	}
	return mem_data_read_byte(address, true);
}

unsigned int cpu_read_word(unsigned int address)
{
	// Program
	if(g_fc & 2) {
		return mem_pgrm_read_word(address);
	}

	/* Otherwise it's data space */
	switch(address) {
		case INPUT_ADDRESS:
			return input_device_read();
		case OUTPUT_ADDRESS:
			return output_device_read();
		default:
			break;
	}
	return mem_data_read_word(address);
}

unsigned int cpu_read_long(unsigned int address) {
	// Program
	if(g_fc & 2) {
		return mem_pgrm_read_long(address);
	}

	/* Otherwise it's data space */
	switch(address) {
		case INPUT_ADDRESS:
			return input_device_read();
		case OUTPUT_ADDRESS:
			return output_device_read();
		default:
			break;
	}
	return mem_data_read_long(address);
}

// Write
////////////////////////////
void cpu_write_byte(unsigned int address, unsigned int value) {
	if(g_fc & 2)	/* Program */
		exit_error("Attempted to write %02x to ROM address %08x", value&0xff, address);

	/* Otherwise it's data space */
	switch(address) {
		case INPUT_ADDRESS:
			input_device_write(value&0xff);
			return;
		case OUTPUT_ADDRESS:
			output_device_write(value&0xff);
			return;
		default:
			break;
	}
	if(address > C2503_RAM_SIZE)
		exit_error("Attempted to write %02x to RAM address %08x", value&0xff, address);
	WRITE_BYTE(g_ram, address, value);
}

void cpu_write_word(unsigned int address, unsigned int value) {
	if(g_fc & 2)	/* Program */
		exit_error("Attempted to write %04x to ROM address %08x", value&0xffff, address);

	/* Otherwise it's data space */
	switch(address) {
		case INPUT_ADDRESS:
			input_device_write(value&0xffff);
			return;
		case OUTPUT_ADDRESS:
			output_device_write(value&0xffff);
			return;
		default:
			break;
	}
	if(address > C2503_RAM_SIZE)
		exit_error("Attempted to write %04x to RAM address %08x", value&0xffff, address);
	WRITE_WORD(g_ram, address, value);
}

void cpu_write_long(unsigned int address, unsigned int value) {
	if(g_fc & 2)	/* Program */
		exit_error("Attempted to write %08x to ROM address %08x", value, address);

	/* Otherwise it's data space */
	switch(address) {
		case INPUT_ADDRESS:
			input_device_write(value);
			return;
		case OUTPUT_ADDRESS:
			output_device_write(value);
			return;
		default:
			break;
	}
	if(address > C2503_RAM_SIZE)
		exit_error("Attempted to write %08x to RAM address %08x", value, address);
	WRITE_LONG(g_ram, address, value);
}

// Debugger
//////////////////////////////////////////////////////////////////////////////////////////////
void make_hex(char* buff, unsigned int pc, unsigned int length) {
	char* ptr = buff;

	for(; length>0; length -= 2) {
		sprintf(ptr, "%04x", mem_pgrm_read_word(pc));
		pc += 2;
		ptr += 4;
		if(length > 2)
			*ptr++ = ' ';
	}
}

// Disassembles x lines of code starting from the PC
// x is dependent on the size of the code window
void update_code_display() {
	unsigned int pc;
	unsigned int instr_size;
	unsigned char line_count = 0;
	char buff[100];
	char buff2[100];

	// Check if there's any room to display anything
	if (emu_win_code_rows <= 2) return;

	//pc = mem_pgrm_read_long(4);
	pc = m68k_get_reg(NULL, M68K_REG_PC);

	// Disassemble to code window
	while (line_count < (emu_win_code_rows - 2)) {
		instr_size = m68k_disassemble(buff, pc, C2503_CPU);
		make_hex(buff2, pc, instr_size);
		sprintf(str_tmp_buf, "%08x: %-20s: %-*s", pc, buff2, (EMU_WIN_CODE_COLS_MAX - 32), buff);
		mvwprintw(emu_win_code, (line_count + 1), 2, "%.*s", (emu_win_code_cols - 4), str_tmp_buf);
		pc += instr_size;
		line_count++;
	}
	wrefresh(emu_win_code);
}

// Helper function for the memory display
unsigned char filter_character_byte(unsigned char value) {
	if ((value >= 0x20) && (value <= 0x7e)) return value;
	return '.';
}

// Produces a hex/ascii listing of the data/program space
void update_memory_display() {
	unsigned char byte_count = 0, line_count = 0, tmp_byte;
	unsigned int tmp_mem_addr = emu_mem_dump_start;
	char buff_hex[48], buff_ascii[16];

	// Check if there's any room to display anything
	if ((emu_win_mem_rows <= 2) || (emu_win_mem_cols <= 2)) return;

	while (line_count < (emu_win_mem_rows - 2)) {
		for (byte_count = 0; byte_count < 0x10; byte_count++) {
			if (emu_mem_dump_type) {
				tmp_byte = mem_pgrm_read_byte(tmp_mem_addr, false);
			} else {
				tmp_byte = mem_data_read_byte(tmp_mem_addr, false);
			}
			sprintf(&buff_hex[byte_count * 3], "%02x ", tmp_byte);
			buff_ascii[byte_count] = filter_character_byte(tmp_byte);
			tmp_mem_addr++;
		}

		sprintf(str_tmp_buf, "%08x:   %.48s      %.16s", tmp_mem_addr, buff_hex, buff_ascii);
		mvwprintw(emu_win_mem, (line_count + 1), 2, "%.*s", (emu_win_mem_cols - 4), str_tmp_buf);
		tmp_mem_addr += 0x10;
		line_count++;
	}
	wrefresh(emu_win_mem);
}

// Displays the state of the CPU registers
void update_register_display() {
	char* str_cpu_type = "Invalid";
	switch (m68k_get_reg(NULL, M68K_REG_CPU_TYPE)) {
		case M68K_CPU_TYPE_68000:
			str_cpu_type = "68000";
			break;
		case M68K_CPU_TYPE_68010:
			str_cpu_type = "68010";
			break;
		case M68K_CPU_TYPE_68EC020:
			str_cpu_type = "68EC020";
			break;
		case M68K_CPU_TYPE_68020:
			str_cpu_type = "68020";
			break;
		case M68K_CPU_TYPE_68EC030:
			str_cpu_type = "68EC030";
			break;
		case M68K_CPU_TYPE_68030:
			str_cpu_type = "68030";
			break;
		case M68K_CPU_TYPE_68EC040:
			str_cpu_type = "68EC040";
			break;
		case M68K_CPU_TYPE_68LC040:
			str_cpu_type = "68LC040";
			break;
		case M68K_CPU_TYPE_68040:
			str_cpu_type = "68040";
			break;
		case M68K_CPU_TYPE_SCC68070:
			str_cpu_type = "SCC68070";
			break;
		default:
			break;
	}
	mvwprintw(emu_win_reg, 1, 2, "CPU Registers (%s):", str_cpu_type);
	sprintf(str_tmp_buf, "D0: %08x   D1: %08x   D2: %08x   D3: %08x   D4: %08x   D5: %08x   D6: %08x   D7: %08x", \
				m68k_get_reg(NULL, M68K_REG_D0), m68k_get_reg(NULL, M68K_REG_D1), m68k_get_reg(NULL, M68K_REG_D2), m68k_get_reg(NULL, M68K_REG_D3), \
				m68k_get_reg(NULL, M68K_REG_D4), m68k_get_reg(NULL, M68K_REG_D5), m68k_get_reg(NULL, M68K_REG_D6), m68k_get_reg(NULL, M68K_REG_D7));
	mvwprintw(emu_win_reg, 2, 2, "%.*s", (emu_win_reg_cols - 4), str_tmp_buf);

	sprintf(str_tmp_buf, "A0: %08x   A1: %08x   A2: %08x   A3: %08x   A4: %08x   A5: %08x   A6: %08x   A7: %08x", \
				m68k_get_reg(NULL, M68K_REG_A0), m68k_get_reg(NULL, M68K_REG_A1), m68k_get_reg(NULL, M68K_REG_A2), m68k_get_reg(NULL, M68K_REG_A3), \
				m68k_get_reg(NULL, M68K_REG_A4), m68k_get_reg(NULL, M68K_REG_A5), m68k_get_reg(NULL, M68K_REG_A6), m68k_get_reg(NULL, M68K_REG_A7));
	mvwprintw(emu_win_reg, 3, 2, "%.*s", (emu_win_reg_cols - 4), str_tmp_buf);

	sprintf(str_tmp_buf, "PC: %08x   SR: %08x [%s|%s|%s|%s|0|%s|%s|%s|0|0|0|%s|%s|%s|%s|%s]", m68k_get_reg(NULL, M68K_REG_PC), m68k_get_reg(NULL, M68K_REG_SR), \
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<15) ? "T1" : "  ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<14) ? "T0" : "  ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<13) ? "S" : " ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<12) ? "M" : " ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<10) ? "I2" : "  ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<9) ? "I1" : "  ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<8) ? "I0" : "  ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<4) ? "X" : " ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<3) ? "N" : " ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<2) ? "Z" : " ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<1) ? "V" : " ",
							(m68k_get_reg(NULL, M68K_REG_SR) & 1<<0) ? "C" : " ");
	mvwprintw(emu_win_reg, 4, 2, "%.*s", (emu_win_reg_cols - 4), str_tmp_buf);

	sprintf(str_tmp_buf, "SP: %08x", m68k_get_reg(NULL, M68K_REG_SP));
	if (m68k_get_reg(NULL, M68K_REG_CPU_TYPE) > M68K_CPU_TYPE_68000) {
		sprintf(str_tmp_buf, "%s   USP: %08x", str_tmp_buf, m68k_get_reg(NULL, M68K_REG_USP));
	}
	if (m68k_get_reg(NULL, M68K_REG_CPU_TYPE) > M68K_CPU_TYPE_68010) {
		sprintf(str_tmp_buf, "%s   MSP: %08x   ISP: %08x", str_tmp_buf, m68k_get_reg(NULL, M68K_REG_MSP), m68k_get_reg(NULL, M68K_REG_ISP));
	}
	if (m68k_get_reg(NULL, M68K_REG_CPU_TYPE) > M68K_CPU_TYPE_68000) {
		sprintf(str_tmp_buf, "%s   VBR: %08x", str_tmp_buf, m68k_get_reg(NULL, M68K_REG_VBR));
	}
	mvwprintw(emu_win_reg, 5, 2, "%.*s", (emu_win_reg_cols - 4), str_tmp_buf);

	wrefresh(emu_win_reg);


//        M68K_REG_SFC,           /* Source Function Code */
//        M68K_REG_DFC,           /* Destination Function Code */
//        M68K_REG_CACR,          /* Cache Control Register */
//        M68K_REG_CAAR,          /* Cache Address Register */
//
//        /* Assumed registers */
//        /* These are cheat registers which emulate the 1-longword prefetch
//         * present in the 68000 and 68010.
//         */
//        M68K_REG_PREF_ADDR,     /* Last prefetch address */
//        M68K_REG_PREF_DATA,     /* Last prefetch data */
//
//        /* Convenience registers */
//        M68K_REG_PPC,           /* Previous value in the program counter */
//        M68K_REG_IR,            /* Instruction register */
//        M68K_REG_CPU_TYPE       /* Type of CPU being run */
}

void print_dbg_actions() {
	printw("Q - Quit\t");
	printw("R - Reset\t");
	printw("s - Step\t");
}

void cpu_instr_callback(int pc) {
	(void)pc;
/* The following code would print out instructions as they are executed */
/*
	static char buff[100];
	static char buff2[100];
	static unsigned int pc;
	static unsigned int instr_size;

	pc = m68k_get_reg(NULL, M68K_REG_PC);
	instr_size = m68k_disassemble(buff, pc, C2503_CPU);
	make_hex(buff2, pc, instr_size);
	printf("E %03x: %-20s: %s\n", pc, buff2, buff);
	fflush(stdout);
*/
}

// NCurses
//////////////////////////////////////////////////////////////////////////////////////////////
void emu_win_resize() {
	endwin();
	clear();
	refresh();

	// Get terminal size
	getmaxyx(stdscr, xterm_rows, xterm_cols);

	// Register Panel
	// Make this the domminant panel when screen height is reduced
	if (xterm_rows <= EMU_WIN_REG_ROWS_MAX) {
		emu_win_reg_rows = xterm_rows;
	} else {
		emu_win_reg_rows = EMU_WIN_REG_ROWS_MAX;
	}
	emu_win_reg_y = xterm_rows - emu_win_reg_rows;
	emu_win_reg_cols = xterm_cols;			// Full screen width
	emu_win_reg_x = 0;				// So starts at the begining
	delwin(emu_win_reg);
	emu_win_reg = newwin(emu_win_reg_rows, emu_win_reg_cols, emu_win_reg_y, emu_win_reg_x);
	box(emu_win_reg, 0 , 0);
	wrefresh(emu_win_reg);

	// Code Panel
	// Use the remaining vertical portion of the screen
	if ((xterm_rows - emu_win_reg_rows) > 0) {
		emu_win_code_rows = xterm_rows - emu_win_reg_rows;
	} else {
		emu_win_code_rows = 0;
	}
	if (xterm_cols <= EMU_WIN_CODE_COLS_MAX) {
		emu_win_code_cols = xterm_cols;
	} else {
		emu_win_code_cols = EMU_WIN_CODE_COLS_MAX;
	}
	emu_win_code_y = 0;
	emu_win_code_x = 0;
	delwin(emu_win_code);
	if (emu_win_code_rows > 0) {
		emu_win_code = newwin(emu_win_code_rows, emu_win_code_cols, emu_win_code_y, emu_win_code_x);
		box(emu_win_code, 0 , 0);
		wrefresh(emu_win_code);
	}

	// Memory Panel
	// Use whatevers left
	if ((xterm_rows - emu_win_reg_rows) > 0) {
		emu_win_mem_rows = xterm_rows - emu_win_reg_rows;
	} else {
		emu_win_mem_rows = 0;
	}
	if ((xterm_cols - emu_win_code_cols) > 0) {
		emu_win_mem_cols = xterm_cols - emu_win_code_cols;
	} else {
		emu_win_mem_cols = 0;
	}
	emu_win_mem_y = 0;
	emu_win_mem_x = emu_win_code_cols;
	delwin(emu_win_mem);
	if ((emu_win_mem_rows > 0) && (emu_win_mem_cols > 0)) {
		emu_win_mem = newwin(emu_win_mem_rows, emu_win_mem_cols, emu_win_mem_y, emu_win_mem_x);
		box(emu_win_mem, 0 , 0);
		wrefresh(emu_win_mem);
	}
}

void emu_win_destroy() {
	delwin(emu_win_code);
	delwin(emu_win_dialog);
	delwin(emu_win_mem);
	delwin(emu_win_reg);
}

// Main loop
//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
	FILE*	fhandle;
	int	key_press;
	char	emu_step = 0;

	if (argc != 2) {
		printf("Usage: cisco_2503 <ROM file>\n");
		exit(-1);
	}

	if((fhandle = fopen(argv[1], "rb")) == NULL) {
		printf("Unable to open %s", argv[1]);
		exit(-1);
	}

	if(fread(g_bootrom, 1, C2503_BOOTROM_SIZE + 1, fhandle) <= 0) {
		printf("Error reading %s", argv[1]);
		exit(-1);
	}

	// Init ncurses
	initscr();
	raw();				// Line buffering disabled
	keypad(stdscr, TRUE);		// We get F1, F2 etc..
	noecho();			// Don't echo() while we do getch
	nodelay(stdscr, true);		// Make character reads non-blocking
	curs_set(0);			// Disable cursor
	emu_win_resize();

	// Init 68k core
	m68k_init();
	m68k_set_cpu_type(C2503_CPU);
	m68k_pulse_reset();
	input_device_reset();
	output_device_reset();
	nmi_device_reset();

	g_quit = 0;
	while (!g_quit) {
		update_code_display();
		update_memory_display();
		update_register_display();

		// Get action
		key_press = wgetch(stdscr);
		if (key_press == KEY_RESIZE) {
			// Handle terminal resize
			emu_win_resize();
		} else if (key_press == 'Q') {
			g_quit = 1;
		} else if (key_press == 'r') {
			emu_step = -1;					// Start execution (run)
		} else if (key_press == 'R') {
			m68k_pulse_reset();
		} else if (key_press == 's') {
			emu_step = 1;					// Execute one instruction
		} else if (key_press == 'S') {
			emu_step = 0;					// Stop execution
		}

		if (emu_step != 0) {
			// Values to execute determine the interleave rate.
			// Smaller values allow for more accurate interleaving with multiple
			// devices/CPUs but is more processor intensive.
			// 100000 is usually a good value to start at, then work from there.

			// Note that I am not emulating the correct clock speed!
			m68k_execute(1);
//			output_device_update();
//			input_device_update();
//			nmi_device_update();

			if (emu_step > 0) emu_step--;
		}

		usleep(2000);
	}

	// Destroy ncurses
	emu_win_destroy();
	endwin();

	return 0;
}

