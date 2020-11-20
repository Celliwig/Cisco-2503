#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include "m68k.h"
#include "cisco_2503.h"
#include "cisco_2503_peripherals.h"
#include <ncurses.h>

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
unsigned int	g_quit = 0;						// 1 if we want to quit
unsigned int	g_nmi = 0;						// 1 if nmi pending

int		g_input_device_value = -1;				// Current value in input device

unsigned int	g_output_device_ready = 0;				// 1 if output device is ready
time_t		g_output_device_last_output;				// Time of last char output

unsigned int	g_int_controller_pending = 0;				// list of pending interrupts
unsigned int	g_int_controller_highest_int = 0;			// Highest pending interrupt
unsigned int	g_fc;							// Current function code from CPU

// NCurses interface
unsigned int	emu_mem_dump_start = 0x00000000;			// Address to start memory dump from
unsigned int	emu_breakpoint = 0x00000000;				// Breakpoint address
unsigned char	emu_mem_dump_type = 0;					// Address space to dump
bool		emu_show_duart = false;
char		str_tmp_buf[512];					// Temporary string buffer
WINDOW		*emu_win_dialog = NULL, *emu_win_duart = NULL, *emu_win_code = NULL, \
		*emu_win_mem = NULL, *emu_win_reg = NULL, *emu_win_status = NULL;
unsigned short int xterm_cols, xterm_cols_remain, xterm_rows, xterm_rows_remain;
unsigned short int emu_win_dialog_cols, emu_win_dialog_rows, emu_win_dialog_y, emu_win_dialog_x;
unsigned short int emu_win_duart_cols, emu_win_duart_rows, emu_win_duart_y, emu_win_duart_x;
unsigned short int emu_win_code_cols, emu_win_code_rows, emu_win_code_y, emu_win_code_x;
unsigned short int emu_win_mem_cols, emu_win_mem_rows, emu_win_mem_y, emu_win_mem_x;
unsigned short int emu_win_reg_cols, emu_win_reg_rows, emu_win_reg_y, emu_win_reg_x;
unsigned short int emu_win_status_cols, emu_win_status_rows, emu_win_status_y, emu_win_status_x;

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

// Emulator
////////////////////////////////////////////////////////
/* Called when the CPU pulses the RESET line */
void cpu_pulse_reset(void) {
	io_system_core_init();
	io_duart_core_init();

//	nmi_device_reset();
//	output_device_reset();
//	input_device_reset();
}

bool cpu_real_read_byte(unsigned int address, unsigned int *tmp_int, bool real_read) {
//	// Program
//	if(g_fc & 2) {
//		return mem_pgrm_read_byte(address, true);
//	}

	if (mem_bootrom_read_byte(address, tmp_int)) return true;
	if (mem_nvram_read_byte(address, tmp_int)) return true;
	if (mem_ram_read_byte(address, tmp_int)) return true;
	if (io_68302_read_byte(address, tmp_int)) return true;
	if (io_system_read_byte(address, tmp_int)) return true;
	if (io_counter_read_byte(address, tmp_int)) return true;
	if (io_duart_read_byte(address, tmp_int, real_read)) return true;
	if (io_channela_read_byte(address, tmp_int)) return true;
	if (io_channelb_read_byte(address, tmp_int)) return true;

	*tmp_int = -1;
	return false;
}

unsigned int cpu_read_byte(unsigned int address) {
	unsigned int tmp_int;

	if (cpu_real_read_byte(address, &tmp_int, true)) return tmp_int;

	exit_error("Attempted to read byte from address %08x", address);
}

unsigned int cpu_read_word(unsigned int address) {
	unsigned int tmp_int;

//	// Program
//	if(g_fc & 2) {
//		return mem_pgrm_read_byte(address, true);
//	}

	if (mem_bootrom_read_word(address, &tmp_int)) return tmp_int;
	if (mem_nvram_read_word(address, &tmp_int)) return tmp_int;
	if (mem_ram_read_word(address, &tmp_int)) return tmp_int;
	if (io_68302_read_word(address, &tmp_int)) return tmp_int;
	if (io_system_read_word(address, &tmp_int)) return tmp_int;
	if (io_counter_read_word(address, &tmp_int)) return tmp_int;
	if (io_channela_read_word(address, &tmp_int)) return tmp_int;
	if (io_channelb_read_word(address, &tmp_int)) return tmp_int;

	exit_error("Attempted to read word from address %08x", address);
}

unsigned int cpu_read_long(unsigned int address) {
	unsigned int tmp_int;

//	// Program
//	if(g_fc & 2) {
//		return mem_pgrm_read_byte(address, true);
//	}

	if (mem_bootrom_read_long(address, &tmp_int)) return tmp_int;
	if (mem_nvram_read_long(address, &tmp_int)) return tmp_int;
	if (mem_ram_read_long(address, &tmp_int)) return tmp_int;
	if (io_68302_read_long(address, &tmp_int)) return tmp_int;
	if (io_system_read_long(address, &tmp_int)) return tmp_int;
	if (io_counter_read_long(address, &tmp_int)) return tmp_int;
	if (io_channela_read_long(address, &tmp_int)) return tmp_int;
	if (io_channelb_read_long(address, &tmp_int)) return tmp_int;

	exit_error("Attempted to read long from address %08x", address);
}

void cpu_write_byte(unsigned int address, unsigned int value) {
//	// Program
//	if(g_fc & 2) {
//		return mem_pgrm_read_byte(address, true);
//	}

	if (mem_nvram_write_byte(address, value)) return;
	if (mem_ram_write_byte(address, value)) return;
	if (io_68302_write_byte(address, value)) return;
	if (io_system_write_byte(address, value)) return;
	if (io_counter_write_byte(address, value)) return;
	if (io_duart_write_byte(address, value)) return;
	if (io_channela_write_byte(address, value)) return;
	if (io_channelb_write_byte(address, value)) return;

	exit_error("Attempted to write byte to address %08x", address);
}

void cpu_write_word(unsigned int address, unsigned int value) {
//	// Program
//	if(g_fc & 2) {
//		return mem_pgrm_read_byte(address, true);
//	}

	if (mem_nvram_write_word(address, value)) return;
	if (mem_ram_write_word(address, value)) return;
	if (io_68302_write_word(address, value)) return;
	if (io_system_write_word(address, value)) return;
	if (io_counter_write_word(address, value)) return;
	if (io_channela_write_word(address, value)) return;
	if (io_channelb_write_word(address, value)) return;

	exit_error("Attempted to write word to address %08x", address);
}

void cpu_write_long(unsigned int address, unsigned int value) {
//	// Program
//	if(g_fc & 2) {
//		return mem_pgrm_read_byte(address, true);
//	}

	if (mem_nvram_write_long(address, value)) return;
	if (mem_ram_write_long(address, value)) return;
	if (io_68302_write_long(address, value)) return;
	if (io_system_write_long(address, value)) return;
	if (io_counter_write_long(address, value)) return;
	if (io_channela_write_long(address, value)) return;
	if (io_channelb_write_long(address, value)) return;

	exit_error("Attempted to write long to address %08x", address);
}

// For peripherals that are clocked
void system_tick() {
	io_duart_core_clock_tick();
}

// Debugger
//////////////////////////////////////////////////////////////////////////////////////////////
void make_hex(char* buff, unsigned int pc, unsigned int length) {
	char* ptr = buff;

	for(; length>0; length -= 2) {
		sprintf(ptr, "%04x", cpu_read_word(pc));
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

	box(emu_win_code, 0 , 0);

	//pc = cpu_read_long(4);
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
	wnoutrefresh(emu_win_code);
}

// Helper function for the memory display
unsigned char filter_character_byte(unsigned char value) {
	if ((value >= 0x20) && (value <= 0x7e)) return value;
	return '.';
}

// Produces a hex/ascii listing of the data/program space
void update_memory_display() {
	unsigned char byte_count = 0, line_count = 0, tmp_byte;
	unsigned int tmp_mem_addr, tmp_int;
	char buff_hex[48], buff_ascii[16], *tmp_mem_addr_src;

	// Check if there's any room to display anything
	if ((emu_win_mem_rows <= 2) || (emu_win_mem_cols <= 2)) return;

	switch (emu_mem_dump_type & EMU_WIN_MEM_DISP_MASK) {
		case EMU_WIN_MEM_DISP_A0:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A0);
			tmp_mem_addr_src = "(A0)";
			break;
		case EMU_WIN_MEM_DISP_A1:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A1);
			tmp_mem_addr_src = "(A1)";
			break;
		case EMU_WIN_MEM_DISP_A2:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A2);
			tmp_mem_addr_src = "(A2)";
			break;
		case EMU_WIN_MEM_DISP_A3:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A3);
			tmp_mem_addr_src = "(A3)";
			break;
		case EMU_WIN_MEM_DISP_A4:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A4);
			tmp_mem_addr_src = "(A4)";
			break;
		case EMU_WIN_MEM_DISP_A5:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A5);
			tmp_mem_addr_src = "(A5)";
			break;
		case EMU_WIN_MEM_DISP_A6:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A6);
			tmp_mem_addr_src = "(A6)";
			break;
		case EMU_WIN_MEM_DISP_A7:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_A7);
			tmp_mem_addr_src = "(A7)";
			break;
		case EMU_WIN_MEM_DISP_PC:
			tmp_mem_addr = m68k_get_reg(NULL, M68K_REG_PC);
			tmp_mem_addr_src = "(PC)";
			break;
		case EMU_WIN_MEM_DISP_SELECTED:
			tmp_mem_addr = emu_mem_dump_start;
			tmp_mem_addr_src = NULL;
			break;
		default:
			tmp_mem_addr = 0x00000000;
			tmp_mem_addr_src = "Invalid";
			break;
	}

	box(emu_win_mem, 0 , 0);
	if (tmp_mem_addr_src == NULL) {
		sprintf(str_tmp_buf, " %s: 0x%08x ", (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) ? "Program" : "Data", tmp_mem_addr);
	} else {
		sprintf(str_tmp_buf, " %s: %s ", (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) ? "Program" : "Data", tmp_mem_addr_src);
	}
	mvwprintw(emu_win_mem, 0, 2, "%s", str_tmp_buf);

	while (line_count < (emu_win_mem_rows - 2)) {
		for (byte_count = 0; byte_count < 0x10; byte_count++) {
			cpu_real_read_byte(tmp_mem_addr + byte_count, &tmp_int, false);
			tmp_byte = tmp_int &0xff;
			sprintf(&buff_hex[byte_count * 3], "%02x ", tmp_byte);
			buff_ascii[byte_count] = filter_character_byte(tmp_byte);
		}

		sprintf(str_tmp_buf, "%08x:   %.48s      %.16s", tmp_mem_addr, buff_hex, buff_ascii);
		mvwprintw(emu_win_mem, (line_count + 1), 2, "%.*s", (emu_win_mem_cols - 4), str_tmp_buf);
		tmp_mem_addr += 0x10;
		line_count++;
	}
	wnoutrefresh(emu_win_mem);
}

// Displays the state of the CPU registers
void update_register_display() {
	char* str_cpu_type = "Invalid";

	box(emu_win_reg, 0 , 0);
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

	wnoutrefresh(emu_win_reg);


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

void update_duart_display() {
	mvwprintw(emu_win_duart, 1, ((emu_win_duart_cols - 10) / 2), "DUART Core");

	mvwprintw(emu_win_duart, 3, 2, "Channel A:");
	sprintf(str_tmp_buf, "Mode 1: 0x%02x    Mode 2: 0x%02x    Clock Select: 0x%02x    Command: 0x%02x", \
					io_duart_core_get_reg(ChannelA_Mode1), \
					io_duart_core_get_reg(ChannelA_Mode2), \
					io_duart_core_get_reg(ChannelA_Clock_Select), \
					io_duart_core_get_reg(ChannelA_Command));
	mvwprintw(emu_win_duart, 4, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);
	sprintf(str_tmp_buf, "Status: 0x%02x    TxHR[%u]: 0x%02x    TxSR[%u]: 0x%02x", \
					io_duart_core_get_reg(ChannelA_Status), \
					io_duart_core_get_reg(ChannelA_Tx_Holding_Empty), \
					io_duart_core_get_reg(ChannelA_Tx_Holding), \
					io_duart_core_get_reg(ChannelA_Tx_Shift_Empty), \
					io_duart_core_get_reg(ChannelA_Tx_Shift));
	mvwprintw(emu_win_duart, 5, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);
	sprintf(str_tmp_buf, "RxSR[%u]: 0x%02x    FIFO0[%02x]: 0x%02x    FIFO1[%02x]: 0x%02x    FIFO2[%02x]: 0x%02x", \
					io_duart_core_get_reg(ChannelA_Rx_Shift_Empty), \
					io_duart_core_get_reg(ChannelA_Rx_Shift), \
					io_duart_core_get_reg(ChannelA_Rx_Fifo0_Status), \
					io_duart_core_get_reg(ChannelA_Rx_Fifo0), \
					io_duart_core_get_reg(ChannelA_Rx_Fifo1_Status), \
					io_duart_core_get_reg(ChannelA_Rx_Fifo1), \
					io_duart_core_get_reg(ChannelA_Rx_Fifo2_Status), \
					io_duart_core_get_reg(ChannelA_Rx_Fifo2));
	mvwprintw(emu_win_duart, 6, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	mvwprintw(emu_win_duart, 8, 2, "Channel B:");
	sprintf(str_tmp_buf, "Mode 1: 0x%02x    Mode 2: 0x%02x    Clock Select: 0x%02x    Command: 0x%02x", \
					io_duart_core_get_reg(ChannelB_Mode1), \
					io_duart_core_get_reg(ChannelB_Mode2), \
					io_duart_core_get_reg(ChannelB_Clock_Select), \
					io_duart_core_get_reg(ChannelB_Command));
	mvwprintw(emu_win_duart, 9, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);
	sprintf(str_tmp_buf, "Status: 0x%02x    TxHR[%u]: 0x%02x    TxSR[%u]: 0x%02x", \
					io_duart_core_get_reg(ChannelB_Status), \
					io_duart_core_get_reg(ChannelB_Tx_Holding_Empty), \
					io_duart_core_get_reg(ChannelB_Tx_Holding), \
					io_duart_core_get_reg(ChannelB_Tx_Shift_Empty), \
					io_duart_core_get_reg(ChannelB_Tx_Shift));
	mvwprintw(emu_win_duart, 10, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);
	sprintf(str_tmp_buf, "RxSR[%u]: 0x%02x    FIFO0[%02x]: 0x%02x    FIFO1[%02x]: 0x%02x    FIFO2[%02x]: 0x%02x", \
					io_duart_core_get_reg(ChannelB_Rx_Shift_Empty), \
					io_duart_core_get_reg(ChannelB_Rx_Shift), \
					io_duart_core_get_reg(ChannelB_Rx_Fifo0_Status), \
					io_duart_core_get_reg(ChannelB_Rx_Fifo0), \
					io_duart_core_get_reg(ChannelB_Rx_Fifo1_Status), \
					io_duart_core_get_reg(ChannelB_Rx_Fifo1), \
					io_duart_core_get_reg(ChannelB_Rx_Fifo2_Status), \
					io_duart_core_get_reg(ChannelB_Rx_Fifo2));
	mvwprintw(emu_win_duart, 11, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	mvwprintw(emu_win_duart, 13, 2, "Interrupts:");
	sprintf(str_tmp_buf, "Interrupt Status: 0x%02x    Interrupt Mask: 0x%02x", \
					io_duart_core_get_reg(Interrupt_Status), \
					io_duart_core_get_reg(Interrupt_Mask));
	mvwprintw(emu_win_duart, 14, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	mvwprintw(emu_win_duart, 16, 2, "Misc:");
	sprintf(str_tmp_buf, "Auxiliary Control: 0x%02x", io_duart_core_get_reg(Auxiliary_Control));
	mvwprintw(emu_win_duart, 17, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	mvwprintw(emu_win_duart, 19, 2, "Counter / Timer:");
	sprintf(str_tmp_buf, "Upper: 0x%02x    Upper Preset: 0x%02x    Start Command: 0x%02x", \
					io_duart_core_get_reg(CounterTimer_Upper), \
					io_duart_core_get_reg(CounterTimer_Upper_Preset), \
					io_duart_core_get_reg(CounterTimer_Start_Command));
	mvwprintw(emu_win_duart, 20, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);
	sprintf(str_tmp_buf, "Lower: 0x%02x    Lower Preset: 0x%02x    Stop Command: 0x%02x", \
					io_duart_core_get_reg(CounterTimer_Lower), \
					io_duart_core_get_reg(CounterTimer_Lower_Preset), \
					io_duart_core_get_reg(CounterTimer_Stop_Command));
	mvwprintw(emu_win_duart, 21, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	mvwprintw(emu_win_duart, 23, 2, "Input Port:");
	sprintf(str_tmp_buf, "Port: 0x%02x    Port Change: 0x%02x", \
					io_duart_core_get_reg(Input_Port), \
					io_duart_core_get_reg(Input_Port_Change));
	mvwprintw(emu_win_duart, 24, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	mvwprintw(emu_win_duart, 26, 2, "Output Port:");
	sprintf(str_tmp_buf, "Port Config: 0x%02x    Port Set: 0x%02x    Port Reset: 0x%02x", \
					io_duart_core_get_reg(Output_Port_Configuration), \
					io_duart_core_get_reg(Output_Port_Set), \
					io_duart_core_get_reg(Output_Port_Reset));
	mvwprintw(emu_win_duart, 27, 4, "%.*s", (emu_win_duart_cols - 6), str_tmp_buf);

	redrawwin(emu_win_duart);
	wnoutrefresh(emu_win_duart);
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
// Checks whether a given ASCII character is a valid hexdecimal input
bool is_hex_char(char value) {
	if ((value >= '0') && (value <= '9')) return true;
	if ((value >= 'A') && (value <= 'F')) return true;
	if ((value >= 'a') && (value <= 'f')) return true;
	return false;
}

// Converts an ASCII hex character to it's equivalent value
unsigned char ascii_2_hex(char value) {
	if ((value >= '0') && (value <= '9')) return (value - '0');
	if ((value >= 'A') && (value <= 'F')) return (10 + (value - 'A'));
	if ((value >= 'a') && (value <= 'f')) return (10 + (value - 'a'));
	return -1;
}

// Draws, with borders, the various windows
// Window creation and size is determined by the overall size of the terminal screen
void emu_win_resize() {
	endwin();
	clear();
	refresh();								// Causes window size to be repopulated

	// Get terminal size
	getmaxyx(stdscr, xterm_rows, xterm_cols);
	xterm_rows_remain = xterm_rows;
	xterm_cols_remain = xterm_cols;

	// Status Panel
	// 1 row at the bottom
	if (xterm_rows <= EMU_WIN_STATUS_ROWS_MAX) {
		emu_win_status_rows = xterm_rows_remain;
	} else {
		emu_win_status_rows = EMU_WIN_STATUS_ROWS_MAX;
	}
	xterm_rows_remain = xterm_rows_remain - emu_win_status_rows;
	emu_win_status_y = xterm_rows_remain;
	emu_win_status_cols = xterm_cols_remain;				// Full screen width
	emu_win_status_x = 0;							// So starts at the begining
	delwin(emu_win_status);
	emu_win_status = newwin(emu_win_status_rows, emu_win_status_cols, emu_win_status_y, emu_win_status_x);
	wnoutrefresh(emu_win_status);

	// Register Panel
	// Make this the domminant panel when screen height is reduced
	if (xterm_rows_remain <= EMU_WIN_REG_ROWS_MAX) {
		emu_win_reg_rows = xterm_rows_remain;
	} else {
		emu_win_reg_rows = EMU_WIN_REG_ROWS_MAX;
	}
	xterm_rows_remain = xterm_rows_remain - emu_win_reg_rows;
	emu_win_reg_y = xterm_rows_remain;
	emu_win_reg_cols = xterm_cols_remain;					// Full screen width
	emu_win_reg_x = 0;							// So starts at the begining
	delwin(emu_win_reg);
	emu_win_reg = newwin(emu_win_reg_rows, emu_win_reg_cols, emu_win_reg_y, emu_win_reg_x);
	box(emu_win_reg, 0 , 0);
	wnoutrefresh(emu_win_reg);

	// Code Panel
	// Use the remaining vertical portion of the screen
	emu_win_code_rows = xterm_rows_remain;
	if (xterm_cols_remain <= EMU_WIN_CODE_COLS_MAX) {
		emu_win_code_cols = xterm_cols_remain;
	} else {
		emu_win_code_cols = EMU_WIN_CODE_COLS_MAX;
	}
	xterm_cols_remain = xterm_cols_remain - emu_win_code_cols;
	emu_win_code_y = 0;
	emu_win_code_x = 0;
	delwin(emu_win_code);
	if (emu_win_code_rows > 0) {
		emu_win_code = newwin(emu_win_code_rows, emu_win_code_cols, emu_win_code_y, emu_win_code_x);
		box(emu_win_code, 0 , 0);
		wnoutrefresh(emu_win_code);
	}

	// Memory Panel
	// Use whatevers left
	emu_win_mem_rows = xterm_rows_remain;
	emu_win_mem_cols = xterm_cols_remain;
	emu_win_mem_y = 0;
	emu_win_mem_x = emu_win_code_cols;
	delwin(emu_win_mem);
	if ((emu_win_mem_rows > 0) && (emu_win_mem_cols > 0)) {
		emu_win_mem = newwin(emu_win_mem_rows, emu_win_mem_cols, emu_win_mem_y, emu_win_mem_x);
		box(emu_win_mem, 0 , 0);
		wnoutrefresh(emu_win_mem);
	}

	// DUART Panel
	// Half screen size
	if (xterm_rows < EMU_WIN_DUART_ROWS_MAX) {
		emu_win_duart_rows = xterm_rows;
	} else {
		emu_win_duart_rows = EMU_WIN_DUART_ROWS_MAX;
	}
	if (xterm_cols < EMU_WIN_DUART_COLS_MAX) {
		emu_win_duart_cols = xterm_cols;
	} else {
		emu_win_duart_cols = EMU_WIN_DUART_COLS_MAX;
	}
	emu_win_duart_y = (xterm_rows - emu_win_duart_rows) / 2;
	emu_win_duart_x = (xterm_cols - emu_win_duart_cols) / 2;
	delwin(emu_win_duart);
	emu_win_duart = newwin(emu_win_duart_rows, emu_win_duart_cols, emu_win_duart_y, emu_win_duart_x);
	box(emu_win_duart, 0 , 0);
	wnoutrefresh(emu_win_duart);
}

// Prints a message in the status window
void emu_status_message(char *message) {
	werase(emu_win_status);
	wprintw(emu_win_status, " %-*s", (emu_win_status_cols - 2), message);
	wnoutrefresh(emu_win_status);
}

// Uses the status window to display a message and input an address
unsigned char emu_win_status_ask_4_address(char *msg, unsigned int *store) {
	bool		input_loop = true;
	char		hex_addr[8];
	unsigned char	hex_digit_index, i, rtn = 0;
	int		key_press;
	unsigned int	tmp_addr = 0;

	nodelay(stdscr, false);							// Temporarily make key scanning blocking

	// Clear digit store
	for (i = 0; i < 8; i++) {
		hex_addr[i] = ' ';
	}
	hex_digit_index = 0;							// Reset index

	while (input_loop) {
		werase(emu_win_status);
		wprintw(emu_win_status, " %s: 0x%-*.8s", msg, (emu_win_status_cols - 14), hex_addr);
		wrefresh(emu_win_status);

		key_press = wgetch(stdscr);
		if (key_press == KEY_RESIZE) {
			emu_win_resize();					// Handle terminal resize
		} else if ((key_press == KEY_ENTER) || (key_press == '\n')) {
			if (hex_digit_index > 0) {
				// Convert characters to integer address
				for (i = 0; i < hex_digit_index; i++) {
					if (i > 0) tmp_addr = tmp_addr << 4;
					tmp_addr += ascii_2_hex(hex_addr[i]);
				}
				rtn = 1;
				*store = tmp_addr;
				input_loop = false;
			}
		} else if (key_press == 0x1b) {					// Escape
			rtn = 0;
			input_loop = false;
		} else if (key_press == KEY_BACKSPACE) {
			// Decrement index if not first digit
			// Or unset last digit
			if (hex_digit_index > 0) {
				hex_digit_index--;
			}
			hex_addr[hex_digit_index] = ' ';
		} else if (is_hex_char(key_press)) {
			if (hex_digit_index < 8) {				// Check that there's room
				hex_addr[hex_digit_index] = key_press;		// Save hex character in store
				hex_digit_index++;				// Increment index
			}
		}
	}

	nodelay(stdscr, true);							// Make key scanning non-blocking again
	return rtn;
}

// Uses the status window to accept an address to set as a breakpoint
void emu_set_breakpoint() {
	if (emu_win_status_ask_4_address("Breakpoint Addr", &emu_breakpoint)) {
		sprintf(str_tmp_buf, "Breakpoint set: 0x%x", emu_breakpoint);
		emu_status_message(str_tmp_buf);
	} else {
		emu_status_message("Canceled");
	}
}

// Uses the status window to accept an address for the memory window
void emu_set_memory_dump_addr() {
	if (emu_win_status_ask_4_address("Memory Addr", &emu_mem_dump_start)) {
		emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_SELECTED;
	} else {
		emu_status_message("Canceled");
	}
}

// Help screen
//void emu_dbg_actions() {
//	printw("Q - Quit\t");
//	printw("R - Reset\t");
//	printw("s - Step\t");
//}

// Destroys all created windows
void emu_win_destroy() {
	delwin(emu_win_code);
	delwin(emu_win_dialog);
	delwin(emu_win_duart);
	delwin(emu_win_mem);
	delwin(emu_win_reg);
	delwin(emu_win_status);
}

// Print usage
//////////////////////////////////////////////////////////////////////////////////////////////
void print_usage() {
	printf("Usage: cisco_2503 -b <ROM file> -s <serial device> \n");
	printf("	-b <ROM file>		Initial boot ROM file.\n");
	printf("	-s <serial device>	Device to connect to, to provide serial emulation.\n");
	exit(-1);
}

// Main loop
//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
	FILE*		fh_bootrom;
	int		fd_serial, key_press, tmp_opt;
	char		emu_step = 0, *bootrom_filename = NULL, *console_devname = NULL;
	struct termios	serial_fd_opts;

	opterr = 0;
	while ((tmp_opt = getopt(argc, argv, "b:s:")) != -1) {
		switch (tmp_opt) {
			case 'b':
				bootrom_filename = optarg;
				break;
			case 's':
				console_devname = optarg;
				break;
			default:
				print_usage();
				break;
		}
	}

	// Check if all needed paramters supplied
	if (bootrom_filename == NULL) {
		print_usage();
	}

	if ((fh_bootrom = fopen(bootrom_filename, "rb")) == NULL) {
		printf("Unable to open %s\n", bootrom_filename);
		exit(-1);
	}
	if (!mem_bootrom_init(fh_bootrom)) {
		printf("Error reading %s\n", bootrom_filename);
		exit(-1);
	}

	// Open console serial device, if it's been specified
	if (console_devname != NULL) {
		if ((fd_serial = open(console_devname, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
			printf("Unable to open %s\n", console_devname);
			exit(-1);
		} else {
			/* get the current options */
			tcgetattr(fd_serial, &serial_fd_opts);

			/* set raw input, 1 second timeout */
			serial_fd_opts.c_cflag |= (CLOCAL | CREAD);
			serial_fd_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			serial_fd_opts.c_oflag &= ~OPOST;
			serial_fd_opts.c_cc[VMIN] = 0;
			serial_fd_opts.c_cc[VTIME] = 0;

			/* set the options */
			tcsetattr(fd_serial, TCSANOW, &serial_fd_opts);

			io_duart_core_channelA_serial_device(fd_serial);
		}
	}

	// Init ncurses
	initscr();
	raw();									// Line buffering disabled
	keypad(stdscr, TRUE);							// We get F1, F2 etc..
	noecho();								// Don't echo() while we do getch
	nodelay(stdscr, true);							// Make character reads non-blocking
	curs_set(0);								// Disable cursor
	emu_win_resize();
	emu_status_message("Cisco 2503 Emulator");

	// Init 68k core
	m68k_init();
	m68k_set_cpu_type(C2503_CPU);
	cpu_pulse_reset();							// Resets I/O subsystems
	m68k_pulse_reset();

	g_quit = 0;
	while (!g_quit) {
		update_code_display();
		update_memory_display();
		update_register_display();
		if (emu_show_duart) update_duart_display();
		doupdate();

		// Check if breakpoint reached
		if (emu_breakpoint == m68k_get_reg(NULL, M68K_REG_PC)) {
			emu_status_message("Breakpoint Reached");
			emu_step = 0;						// Stop execution
		}

		// Get action
		key_press = wgetch(stdscr);
		if (key_press == KEY_RESIZE) {
			// Handle terminal resize
			emu_win_resize();
		} else if (key_press == 'b') {
			emu_set_breakpoint();
		} else if (key_press == 'Q') {
			g_quit = 1;
		} else if (key_press == 'r') {
			if (emu_step == 0) {
				emu_status_message("Running");
				emu_step = -1;					// Start execution (run)
			} else {
				emu_status_message("Stopped");
				emu_step = 0;					// Stop execution
			}
		} else if (key_press == 'R') {
			emu_status_message("Reset");
			cpu_pulse_reset();					// Resets I/O subsystems
			m68k_pulse_reset();
		} else if (key_press == 's') {
			emu_status_message("Stepped");
			emu_step = 1;						// Execute one instruction
//		} else if (key_press == 'S') {
		} else if (key_press == 'U') {
			emu_show_duart = emu_show_duart ^ true;			// Toggle DUART window
		} else if (key_press == 'd') {
			// Memory window displays data
			emu_mem_dump_type = emu_mem_dump_type & EMU_WIN_MEM_DISP_MASK;
		} else if (key_press == 'p') {
			// Memory window displays program
			emu_mem_dump_type = emu_mem_dump_type | EMU_WIN_MEM_DISP_SPACE;
		} else if (key_press == 'm') {
			emu_set_memory_dump_addr();
		} else if (key_press == '0') {
			// Memory window displays address from A0
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A0;
		} else if (key_press == '1') {
			// Memory window displays address from A1
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A1;
		} else if (key_press == '2') {
			// Memory window displays address from A2
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A2;
		} else if (key_press == '3') {
			// Memory window displays address from A3
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A3;
		} else if (key_press == '4') {
			// Memory window displays address from A4
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A4;
		} else if (key_press == '5') {
			// Memory window displays address from A5
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A5;
		} else if (key_press == '6') {
			// Memory window displays address from A6
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A6;
		} else if (key_press == '7') {
			// Memory window displays address from A7
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_A7;
		} else if (key_press == '8') {
			// Memory window displays address from PC
			emu_mem_dump_type = (emu_mem_dump_type & EMU_WIN_MEM_DISP_SPACE) | EMU_WIN_MEM_DISP_PC;
		}

		if (emu_step != 0) {
			// Values to execute determine the interleave rate.
			// Smaller values allow for more accurate interleaving with multiple
			// devices/CPUs but is more processor intensive.
			// 100000 is usually a good value to start at, then work from there.

			// Note that I am not emulating the correct clock speed!
			m68k_execute(1);
//			nmi_device_update();
			system_tick();

			if (emu_step > 0) emu_step--;
		}

		usleep(800);
	}

	// Destroy ncurses
	emu_win_destroy();
	endwin();

	return 0;
}

