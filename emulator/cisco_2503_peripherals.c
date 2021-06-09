#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
//#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "m68k.h"
#include "cisco_2503.h"
#include "cisco_2503_peripherals.h"

// System Register Intergrator
//////////////////////////////////////////////////////////////////////////////////////////////
bool	sri_enabled = false;					// Flag for System Register Integrator
int	sri_fd = -1;						// File descriptor for SRI
char	*sri_logfile = "cisco_2503.sri.log";			// SRI logfile
bool	sri_logfile_enabled = false;
int	sri_logfile_fh = -1;

void disableSRI() { sri_enabled = false; }
void enableSRI() { sri_enabled = true; }
int getSRIFD() { return sri_fd; }
void setSRIFD(int fd) { sri_fd = fd; }
bool statusSRI() { return sri_enabled; }

void disableSRILogging() {
	sri_logfile_enabled = false;
	if (sri_logfile_fh != -1) {
		fsync(sri_logfile_fh);
		close(sri_logfile_fh);
		sri_logfile_fh = -1;
	}
}
void enableSRILogging() {
	sri_logfile_enabled = true;
	sri_logfile_fh = open(sri_logfile, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}
bool statusSRILogging() { return sri_logfile_enabled; }
void writeSRILog(char *sri_request, char *sri_response) {
	char log_buffer[(SRI_BUFFER_SIZE * 2) + 0x10];			/* Buffer sized to maximum size of request and response messages */
									/* Plus a bit extra for spacing */

	if (sri_logfile_enabled && (sri_logfile_fh != -1)) {
		sprintf(&log_buffer[0], "%s - %s\n", sri_request, sri_response);
		write(sri_logfile_fh, &log_buffer[0], strlen(&log_buffer[0]));
	}
}

/* Use SRI to read a byte of data from specified address */
bool sriReadRequest(unsigned int address, unsigned char op_width, unsigned int *value) {
	bool		loop = true;
	unsigned char	char_buffer = 0, char_index = 0, response_header;
	unsigned int	response_address, response_width, response_buserror, response_data;
	char	sri_request[SRI_BUFFER_SIZE];				// SRI request buffer
	char	sri_response[SRI_BUFFER_SIZE];				// SRI response buffer
	char	*str_eol = "\r\n";

	if (sri_enabled && (sri_fd >= 0)) {
		/* Prep SRI request */
		sprintf(&sri_request[0], "R%08x%02x", address, op_width);
		/* Write out request */
		write(sri_fd, &sri_request, strlen(&sri_request[0]));
		write(sri_fd, str_eol, strlen(str_eol));

		/* Read response */
		while (loop) {
			if (read(sri_fd, &char_buffer, 1) == 1) {
				/* Ignore carriage return/newline while buffer empty */
				if ((char_buffer  == '\r') || (char_buffer == '\n')) {
					if (char_index > 0) {
						/* Add NULL character for SRI logging */
						sri_response[char_index] = 0;
						loop = false;
					}
				} else {
					/* Check there's space in buffer */
					if (char_index < SRI_BUFFER_SIZE) {
						sri_response[char_index] = char_buffer;
						char_index++;
					}
				}
			}
		}

		/* Log request/response */
		writeSRILog(&sri_request, &sri_response);

		/* Process response */
		/* First, check response length */
		switch (op_width) {
			case 1:
				if (char_index != 15) return false;
				break;
			case 2:
				if (char_index != 17) return false;
				break;
			case 4:
				if (char_index != 21) return false;
				break;
			default:
				return false;
				break;
		}
		/* Parse response */
		if (sscanf(&sri_response[0], "%c%8x%2x%2x%8x", &response_header, &response_address, &response_width, &response_buserror, &response_data) == 5) {
			if ((response_header == 'R') && (response_address == address) && (response_width == op_width)) {
				if (response_buserror) {
					m68k_pulse_bus_error();
					return false;
				} else {
					*value = response_data;
					return true;
				}
			}
		}
	}

	return false;
}

bool sriReadByte(unsigned int address, unsigned int *value) { return sriReadRequest(address, 1, value); }
bool sriReadWord(unsigned int address, unsigned int *value) { return sriReadRequest(address, 2, value); }
bool sriReadLong(unsigned int address, unsigned int *value) { return sriReadRequest(address, 4, value); }

/* Use SRI to write a byte of data to the specified address */
bool sriWriteRequest(unsigned int address, unsigned char op_width, unsigned int value) {
	bool		loop = true;
	unsigned char	char_buffer = 0, char_index = 0, response_header;
	unsigned int	response_address, response_width, response_buserror, response_data;
	char	sri_request[SRI_BUFFER_SIZE];				// SRI request buffer
	char	sri_response[SRI_BUFFER_SIZE];				// SRI response buffer
	char	*str_eol = "\r\n";

	if (sri_enabled && (sri_fd >= 0)) {
		/* Prep SRI request */
		sprintf(&sri_request[0], "W%08x%02x%0*x", address, op_width, (op_width * 2), value);
		/* Write out request */
		write(sri_fd, &sri_request, strlen(&sri_request[0]));
		write(sri_fd, str_eol, strlen(str_eol));

		/* Read response */
		while (loop) {
			if (read(sri_fd, &char_buffer, 1) == 1) {
				/* Ignore carriage return/newline while buffer empty */
				if ((char_buffer  == '\r') || (char_buffer == '\n')) {
					if (char_index > 0) {
						/* Add NULL character for SRI logging */
						sri_response[char_index] = 0;
						loop = false;
					}
				} else {
					/* Check there's space in buffer */
					if (char_index < SRI_BUFFER_SIZE) {
						sri_response[char_index] = char_buffer;
						char_index++;
					}
				}
			}
		}

		/* Log request/response */
		writeSRILog(&sri_request, &sri_response);

		/* Process response */
		/* First, check response length */
		switch (op_width) {
			case 1:
				if (char_index != 15) return false;
				break;
			case 2:
				if (char_index != 17) return false;
				break;
			case 4:
				if (char_index != 21) return false;
				break;
			default:
				return false;
				break;
		}
		/* Parse response */
		if (sscanf(&sri_response[0], "%c%8x%2x%2x%8x", &response_header, &response_address, &response_width, &response_buserror, &response_data) == 5) {
			if ((response_header == 'W') && (response_address == address) && (response_width == op_width) && (response_data == value)) {
				if (response_buserror) {
					m68k_pulse_bus_error();
					return false;
				} else {
					return true;
				}
			}
		}
	}

	return false;
}

bool sriWriteByte(unsigned int address, unsigned int value) { return sriWriteRequest(address, 1, value); }
bool sriWriteWord(unsigned int address, unsigned int value) { return sriWriteRequest(address, 2, value); }
bool sriWriteLong(unsigned int address, unsigned int value) { return sriWriteRequest(address, 4, value); }

// System Registers
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char		g_io_sysid_cookie[C2503_IO_SYS_ID_COOKIE_SIZE];
unsigned char		g_io_sysid_cookie2, g_x24c44_cmd_reg;
unsigned short int	g_x24c44_shift_reg;

// PROM Cookie (taken from hardware)
// 0x0b, 0x01, 0x00, 0xe0, 0x1e, 0xb9, 0x23, 0x91, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
// 0x07, 0x00, 0x33, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
unsigned short cookie_prom_data[16] = { 0x0b01, 0x0700, 0x0600, 0x0000, 0x1eb9, 0x0000, 0x0100, 0x0000, \
					0x00e0, 0x3370, 0x0000, 0x0000, 0x2391, 0x0000, 0x0000, 0x0000 };

bool
			/* System control register (Offset: 0x00) */
			g_io_sysctrl00_0_unkwn, \
			g_io_sysctrl00_1_unkwn, \
			g_io_sysctrl00_2_unkwn, \
			g_io_sysctrl00_3_unkwn, \
			g_io_sysctrl00_4_unkwn, \
			g_io_sysctrl00_5_unkwn, \
			g_io_sysctrl00_6_unkwn, \
			g_io_sysctrl00_7_unkwn, \

			// System control register (Offset: 0x01)
			g_io_sysctrl01_0_remap_rom, \
			g_io_sysctrl01_1_unkwn, \
			g_io_sysctrl01_2_unkwn, \
			g_io_sysctrl01_3_unkwn, \
			g_io_sysctrl01_4_status_led, \
			g_io_sysctrl01_5_unkwn, \
			g_io_sysctrl01_6_unkwn, \
			g_io_sysctrl01_7_unkwn, \

			// System control register (Offset: 0x03)
			g_io_sysctrl03_0_prom_bw0, \
			g_io_sysctrl03_1_prom_bw1, \
			g_io_sysctrl03_2_prom_ws0, \
			g_io_sysctrl03_3_prom_ws1, \
			g_io_sysctrl03_4_ram_ws0, \
			g_io_sysctrl03_5_ram_ws1, \
			g_io_sysctrl03_6_unkwn, \
			g_io_sysctrl03_7_unkwn, \

			// System control register (Offset: 0x04)
			g_io_sysctrl04_0_unkwn, \
			g_io_sysctrl04_1_unkwn, \
			g_io_sysctrl04_2_unkwn, \
			g_io_sysctrl04_3_unkwn, \
			g_io_sysctrl04_4_unkwn, \
			g_io_sysctrl04_5_unkwn, \
			g_io_sysctrl04_6_unkwn, \
			g_io_sysctrl04_7_unkwn, \

			// System control register (Offset: 0x05)
			g_io_sysctrl05_0_unkwn, \
			g_io_sysctrl05_1_reset_channela, \
			g_io_sysctrl05_2_reset_channelb, \
			g_io_sysctrl05_3_reset_68302, \
			g_io_sysctrl05_4_unkwn, \
			g_io_sysctrl05_5_unkwn, \
			g_io_sysctrl05_6_unkwn, \
			g_io_sysctrl05_7_unkwn, \

			// System control register (Offset: 0x06)
			g_io_sysctrl06_0_unkwn, \
			g_io_sysctrl06_1_unkwn, \
			g_io_sysctrl06_2_unkwn, \
			g_io_sysctrl06_3_unkwn, \
			g_io_sysctrl06_4_unkwn, \
			g_io_sysctrl06_5_unkwn, \
			g_io_sysctrl06_6_unkwn, \
			g_io_sysctrl06_7_unkwn, \

			// System control register (Offset: 0x07)
/*			g_io_sysctrl07_0_unkwn, \
			g_io_sysctrl07_1_unkwn, \
			g_io_sysctrl07_2_unkwn, \
			g_io_sysctrl07_3_unkwn, \
			g_io_sysctrl07_4_unkwn, \
			g_io_sysctrl07_5_unkwn, \
			g_io_sysctrl07_6_unkwn, \
			g_io_sysctrl07_7_unkwn, \ */

			// System control register (Offset: 0x08)
			g_io_sysctrl08_0_unkwn, \
			g_io_sysctrl08_1_unkwn, \
			g_io_sysctrl08_2_unkwn, \
			g_io_sysctrl08_3_unkwn, \
			g_io_sysctrl08_4_unkwn, \
			g_io_sysctrl08_5_unkwn, \
			g_io_sysctrl08_6_unkwn, \
			g_io_sysctrl08_7_unkwn, \

			// System control register (Offset: 0x09)
			g_io_sysctrl09_0_unkwn, \
			g_io_sysctrl09_1_unkwn, \
			g_io_sysctrl09_2_unkwn, \
			g_io_sysctrl09_3_unkwn, \
			g_io_sysctrl09_4_unkwn, \
			g_io_sysctrl09_5_unkwn, \
			g_io_sysctrl09_6_unkwn, \
			g_io_sysctrl09_7_unkwn, \

			// System control register (Offset: 0x0a)
			g_io_sysctrl0a_0_unkwn, \
			g_io_sysctrl0a_1_unkwn, \
			g_io_sysctrl0a_2_unkwn, \
			g_io_sysctrl0a_3_unkwn, \
			g_io_sysctrl0a_4_unkwn, \
			g_io_sysctrl0a_5_unkwn, \
			g_io_sysctrl0a_6_unkwn, \
			g_io_sysctrl0a_7_unkwn, \

			// System control register (Offset: 0x0b)
			g_io_sysctrl0b_0_unkwn, \
			g_io_sysctrl0b_1_unkwn, \
			g_io_sysctrl0b_2_unkwn, \
			g_io_sysctrl0b_3_unkwn, \
			g_io_sysctrl0b_4_unkwn, \
			g_io_sysctrl0b_5_unkwn, \
			g_io_sysctrl0b_6_unkwn, \
			g_io_sysctrl0b_7_unkwn;


// Initialise (reset) system control core
//////////////////////////////////////////////////////////////////////////////////////////////
void io_system_core_init() {
	// Init System ID Cookie
	for (int i = 0; i < 0x20; i++) {
		g_io_sysid_cookie[i] = 0xff;
	}
	// Init alternate System ID Cookie subsystem
	g_io_sysid_cookie2 = 0x03;
	g_x24c44_cmd_reg = 0x00;
	g_x24c44_shift_reg = 0x0000;

	// Configure control registers
	io_system_cntl_reg_write(0x00, 0x00);
	io_system_cntl_reg_write(0x01, 0x01);				// Map boot ROM to 0x00000000
	io_system_cntl_reg_write(0x03, 0x00);
	io_system_cntl_reg_write(0x04, 0x00);
	io_system_cntl_reg_write(0x05, 0x3F);
	io_system_cntl_reg_write(0x06, 0x00);
	io_system_cntl_reg_write(0x07, 0x00);
	io_system_cntl_reg_write(0x08, 0x41);
	io_system_cntl_reg_write(0x09, 0xFA);
	io_system_cntl_reg_write(0x0a, 0x6F);
	io_system_cntl_reg_write(0x0b, 0x40);
}

bool io_system_cntl_reg_read(unsigned char address_offset, unsigned char *value) {
	*value = 0;

	switch (address_offset) {
		case 0x00:
			if (g_io_sysctrl00_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl00_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl00_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl00_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl00_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl00_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl00_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl00_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x01:
			if (g_io_sysctrl01_0_remap_rom) *value |= 0x01;
			if (g_io_sysctrl01_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl01_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl01_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl01_4_status_led) *value |= 0x10;
			if (g_io_sysctrl01_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl01_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl01_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x02:
			*value = C2503_IO_SYS_CONTROL_HW_VERSION;
			return true;
			break;
		case 0x03:
			if (g_io_sysctrl03_0_prom_bw0) *value |= 0x01;
			if (g_io_sysctrl03_1_prom_bw1) *value |= 0x02;
			if (g_io_sysctrl03_2_prom_ws0) *value |= 0x04;
			if (g_io_sysctrl03_3_prom_ws1) *value |= 0x08;
			if (g_io_sysctrl03_4_ram_ws0) *value |= 0x10;
			if (g_io_sysctrl03_5_ram_ws1) *value |= 0x20;
			if (g_io_sysctrl03_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl03_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x04:
			if (g_io_sysctrl04_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl04_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl04_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl04_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl04_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl04_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl04_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl04_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x05:
			if (g_io_sysctrl05_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl05_1_reset_channela) *value |= 0x02;
			if (g_io_sysctrl05_2_reset_channelb) *value |= 0x04;
			if (g_io_sysctrl05_3_reset_68302) *value |= 0x08;
			if (g_io_sysctrl05_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl05_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl05_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl05_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x06:
			if (g_io_sysctrl06_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl06_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl06_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl06_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl06_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl06_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl06_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl06_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x07:
			if (io_channela_isIRQ()) *value |= 0x02;
			if (io_channelb_isIRQ()) *value |= 0x04;
			if (io_68302_isIRQ()) *value |= 0x08;
			return true;
			break;
		case 0x08:
			if (g_io_sysctrl08_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl08_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl08_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl08_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl08_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl08_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl08_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl08_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x09:
			if (g_io_sysctrl09_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl09_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl09_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl09_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl09_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl09_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl09_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl09_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x0a:
			if (g_io_sysctrl0a_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl0a_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl0a_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl0a_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl0a_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl0a_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl0a_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl0a_7_unkwn) *value |= 0x80;
			return true;
			break;
		case 0x0b:
			if (g_io_sysctrl0b_0_unkwn) *value |= 0x01;
			if (g_io_sysctrl0b_1_unkwn) *value |= 0x02;
			if (g_io_sysctrl0b_2_unkwn) *value |= 0x04;
			if (g_io_sysctrl0b_3_unkwn) *value |= 0x08;
			if (g_io_sysctrl0b_4_unkwn) *value |= 0x10;
			if (g_io_sysctrl0b_5_unkwn) *value |= 0x20;
			if (g_io_sysctrl0b_6_unkwn) *value |= 0x40;
			if (g_io_sysctrl0b_7_unkwn) *value |= 0x80;
			return true;
			break;
		default:
			return false;
	}

	return false;
}

bool io_system_cntl_reg_write(unsigned char address_offset, unsigned char value) {
	switch (address_offset) {
		case 0x00:
			if (value & 0x01) { g_io_sysctrl00_0_unkwn = true; } else { g_io_sysctrl00_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl00_1_unkwn = true; } else { g_io_sysctrl00_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl00_2_unkwn = true; } else { g_io_sysctrl00_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl00_3_unkwn = true; } else { g_io_sysctrl00_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl00_4_unkwn = true; } else { g_io_sysctrl00_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl00_5_unkwn = true; } else { g_io_sysctrl00_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl00_6_unkwn = true; } else { g_io_sysctrl00_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl00_7_unkwn = true; } else { g_io_sysctrl00_7_unkwn = false; }
			return true;
			break;
		case 0x01:
			if (value & 0x01) { g_io_sysctrl01_0_remap_rom = true; } else { g_io_sysctrl01_0_remap_rom = false; }
			if (value & 0x02) { g_io_sysctrl01_1_unkwn = true; } else { g_io_sysctrl01_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl01_2_unkwn = true; } else { g_io_sysctrl01_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl01_3_unkwn = true; } else { g_io_sysctrl01_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl01_4_status_led = true; } else { g_io_sysctrl01_4_status_led = false; }
			if (value & 0x20) { g_io_sysctrl01_5_unkwn = true; } else { g_io_sysctrl01_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl01_6_unkwn = true; } else { g_io_sysctrl01_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl01_7_unkwn = true; } else { g_io_sysctrl01_7_unkwn = false; }
			return true;
			break;
		case 0x02:
			// Hardware version register read-only
			return true;
			break;
		case 0x03:
			if (value & 0x01) { g_io_sysctrl03_0_prom_bw0 = true; } else { g_io_sysctrl03_0_prom_bw0 = false; }
			if (value & 0x02) { g_io_sysctrl03_1_prom_bw1 = true; } else { g_io_sysctrl03_1_prom_bw1 = false; }
			if (value & 0x04) { g_io_sysctrl03_2_prom_ws0 = true; } else { g_io_sysctrl03_2_prom_ws0 = false; }
			if (value & 0x08) { g_io_sysctrl03_3_prom_ws1 = true; } else { g_io_sysctrl03_3_prom_ws1 = false; }
			if (value & 0x10) { g_io_sysctrl03_4_ram_ws0 = true; } else { g_io_sysctrl03_4_ram_ws0 = false; }
			if (value & 0x20) { g_io_sysctrl03_5_ram_ws1 = true; } else { g_io_sysctrl03_5_ram_ws1 = false; }
			if (value & 0x40) { g_io_sysctrl03_6_unkwn = true; } else { g_io_sysctrl03_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl03_7_unkwn = true; } else { g_io_sysctrl03_7_unkwn = false; }
			return true;
			break;
		case 0x04:
			if (value & 0x01) { g_io_sysctrl04_0_unkwn = true; } else { g_io_sysctrl04_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl04_1_unkwn = true; } else { g_io_sysctrl04_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl04_2_unkwn = true; } else { g_io_sysctrl04_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl04_3_unkwn = true; } else { g_io_sysctrl04_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl04_4_unkwn = true; } else { g_io_sysctrl04_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl04_5_unkwn = true; } else { g_io_sysctrl04_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl04_6_unkwn = true; } else { g_io_sysctrl04_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl04_7_unkwn = true; } else { g_io_sysctrl04_7_unkwn = false; }
			return true;
			break;
		case 0x05:
			if (value & 0x01) { g_io_sysctrl05_0_unkwn = true; } else { g_io_sysctrl05_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl05_1_reset_channela = true; } else { g_io_sysctrl05_1_reset_channela = false; }
			if (value & 0x04) { g_io_sysctrl05_2_reset_channelb = true; } else { g_io_sysctrl05_2_reset_channelb = false; }
			if (value & 0x08) { g_io_sysctrl05_3_reset_68302 = true; io_68302_core_init(); } else { g_io_sysctrl05_3_reset_68302 = false; }
			if (value & 0x10) { g_io_sysctrl05_4_unkwn = true; } else { g_io_sysctrl05_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl05_5_unkwn = true; } else { g_io_sysctrl05_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl05_6_unkwn = true; } else { g_io_sysctrl05_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl05_7_unkwn = true; } else { g_io_sysctrl05_7_unkwn = false; }
			return true;
			break;
		case 0x06:
			if (value & 0x01) { g_io_sysctrl06_0_unkwn = true; } else { g_io_sysctrl06_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl06_1_unkwn = true; } else { g_io_sysctrl06_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl06_2_unkwn = true; } else { g_io_sysctrl06_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl06_3_unkwn = true; } else { g_io_sysctrl06_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl06_4_unkwn = true; } else { g_io_sysctrl06_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl06_5_unkwn = true; } else { g_io_sysctrl06_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl06_6_unkwn = true; } else { g_io_sysctrl06_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl06_7_unkwn = true; } else { g_io_sysctrl06_7_unkwn = false; }
			return true;
			break;
		case 0x07:
			// Read only?
			return true;
			break;
		case 0x08:
			if (value & 0x01) { g_io_sysctrl08_0_unkwn = true; } else { g_io_sysctrl08_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl08_1_unkwn = true; } else { g_io_sysctrl08_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl08_2_unkwn = true; } else { g_io_sysctrl08_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl08_3_unkwn = true; } else { g_io_sysctrl08_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl08_4_unkwn = true; } else { g_io_sysctrl08_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl08_5_unkwn = true; } else { g_io_sysctrl08_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl08_6_unkwn = true; } else { g_io_sysctrl08_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl08_7_unkwn = true; } else { g_io_sysctrl08_7_unkwn = false; }
			return true;
			break;
		case 0x09:
			if (value & 0x01) { g_io_sysctrl09_0_unkwn = true; } else { g_io_sysctrl09_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl09_1_unkwn = true; } else { g_io_sysctrl09_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl09_2_unkwn = true; } else { g_io_sysctrl09_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl09_3_unkwn = true; } else { g_io_sysctrl09_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl09_4_unkwn = true; } else { g_io_sysctrl09_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl09_5_unkwn = true; } else { g_io_sysctrl09_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl09_6_unkwn = true; } else { g_io_sysctrl09_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl09_7_unkwn = true; } else { g_io_sysctrl09_7_unkwn = false; }
			return true;
			break;
		case 0x0a:
			if (value & 0x01) { g_io_sysctrl0a_0_unkwn = true; } else { g_io_sysctrl0a_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl0a_1_unkwn = true; } else { g_io_sysctrl0a_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl0a_2_unkwn = true; } else { g_io_sysctrl0a_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl0a_3_unkwn = true; } else { g_io_sysctrl0a_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl0a_4_unkwn = true; } else { g_io_sysctrl0a_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl0a_5_unkwn = true; } else { g_io_sysctrl0a_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl0a_6_unkwn = true; } else { g_io_sysctrl0a_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl0a_7_unkwn = true; } else { g_io_sysctrl0a_7_unkwn = false; }
			return true;
			break;
		case 0x0b:
			if (value & 0x01) { g_io_sysctrl0b_0_unkwn = true; } else { g_io_sysctrl0b_0_unkwn = false; }
			if (value & 0x02) { g_io_sysctrl0b_1_unkwn = true; } else { g_io_sysctrl0b_1_unkwn = false; }
			if (value & 0x04) { g_io_sysctrl0b_2_unkwn = true; } else { g_io_sysctrl0b_2_unkwn = false; }
			if (value & 0x08) { g_io_sysctrl0b_3_unkwn = true; } else { g_io_sysctrl0b_3_unkwn = false; }
			if (value & 0x10) { g_io_sysctrl0b_4_unkwn = true; } else { g_io_sysctrl0b_4_unkwn = false; }
			if (value & 0x20) { g_io_sysctrl0b_5_unkwn = true; } else { g_io_sysctrl0b_5_unkwn = false; }
			if (value & 0x40) { g_io_sysctrl0b_6_unkwn = true; } else { g_io_sysctrl0b_6_unkwn = false; }
			if (value & 0x80) { g_io_sysctrl0b_7_unkwn = true; } else { g_io_sysctrl0b_7_unkwn = false; }
			return true;
			break;
		default:
			return false;
	}

	return false;
}

bool io_system_read_byte(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

	// Main system control registers
	if ((address >= C2503_IO_SYS_CONTROL_BASE_ADDR) && (address < (C2503_IO_SYS_CONTROL_BASE_ADDR + C2503_IO_SYS_CONTROL_WIN_SIZE))) {
		unsigned char tmp_val;
		*value = 0;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR), &tmp_val)) return false;
		*value |= tmp_val;
		return true;
	}

	// System ID cookie (Old interface)
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		*value = READ_BYTE(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR);
		return true;
	}
	// System ID cookie (New interface)
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR2) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR2 + C2503_IO_SYS_ID_COOKIE_SIZE2))) {
		// Bit 0 is the output from X24C44 EEPROM
		*value = (g_io_sysid_cookie2 & 0xfe) | (g_x24c44_shift_reg & 0x0001);
		return true;
	}
	return false;
}

bool io_system_read_word(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

	// Main system control registers
	if ((address >= C2503_IO_SYS_CONTROL_BASE_ADDR) && (address < (C2503_IO_SYS_CONTROL_BASE_ADDR + C2503_IO_SYS_CONTROL_WIN_SIZE))) {
		unsigned char tmp_val;
		*value = 0;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR), &tmp_val)) return false;
		*value |= tmp_val << 8;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 1, &tmp_val)) return false;
		*value |= tmp_val;
		return true;
	}

	return false;
}

bool io_system_read_long(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

	// Main system control registers
	if ((address >= C2503_IO_SYS_CONTROL_BASE_ADDR) && (address < (C2503_IO_SYS_CONTROL_BASE_ADDR + C2503_IO_SYS_CONTROL_WIN_SIZE))) {
		unsigned char tmp_val;
		*value = 0;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR), &tmp_val)) return false;
		*value |= tmp_val << 24;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 1, &tmp_val)) return false;
		*value |= tmp_val << 16;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 2, &tmp_val)) return false;
		*value |= tmp_val << 8;
		if (!io_system_cntl_reg_read((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 3, &tmp_val)) return false;
		*value |= tmp_val;
		return true;
	}

	return false;
}

bool io_system_write_byte(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass write

	// Main system control registers
	if ((address >= C2503_IO_SYS_CONTROL_BASE_ADDR) && (address < (C2503_IO_SYS_CONTROL_BASE_ADDR + C2503_IO_SYS_CONTROL_WIN_SIZE))) {
		unsigned char tmp_val;
		tmp_val = value & 0x000000ff;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR), tmp_val)) return false;
		return true;
	}

	// System ID cookie (Old interface)
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		WRITE_BYTE(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR, value);
		return true;
	}
	// System ID cookie (New interface)
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR2) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR2 + C2503_IO_SYS_ID_COOKIE_SIZE2))) {
		// Check if the PROM is selected
		if (g_io_sysid_cookie2 & 0x08) {
			// Check for clock pulse
			if ((~g_io_sysid_cookie2 & value) & 0x04) {
				// Check whether EEPROM command complete
				if (g_x24c44_cmd_reg & 0x80) {
					// Value in shift register shifted on clock pulse
					g_x24c44_shift_reg = g_x24c44_shift_reg >> 1;
				} else {
					// Shift command value
					g_x24c44_cmd_reg = g_x24c44_cmd_reg << 1;
					// Add bit value
					if (value & 0x02) g_x24c44_cmd_reg |= 1;

					// If command complete, do setup
					if (g_x24c44_cmd_reg & 0x80) {
						// Check for read command
						if ((0x86 & g_x24c44_cmd_reg) == 0x86) {
							// Extract address from command
							// And move data from storage array into shift register
							g_x24c44_shift_reg = cookie_prom_data[(g_x24c44_cmd_reg >> 3) & 0xf];
						}
					}
				}
			}
		} else {
			g_x24c44_cmd_reg = 0x0;
		}
		g_io_sysid_cookie2 = value;
		return true;
	}
	return false;
}

bool io_system_write_word(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass write

	// Main system control registers
	if ((address >= C2503_IO_SYS_CONTROL_BASE_ADDR) && (address < (C2503_IO_SYS_CONTROL_BASE_ADDR + C2503_IO_SYS_CONTROL_WIN_SIZE))) {
		unsigned char tmp_val;
		tmp_val = value & 0x0000ff00;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR), tmp_val)) return false;
		tmp_val = value & 0x000000ff;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 1, tmp_val)) return false;
		return true;
	}

	return false;
}

bool io_system_write_long(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass write

	// Main system control registers
	if ((address >= C2503_IO_SYS_CONTROL_BASE_ADDR) && (address < (C2503_IO_SYS_CONTROL_BASE_ADDR + C2503_IO_SYS_CONTROL_WIN_SIZE))) {
		unsigned char tmp_val;
		tmp_val = value & 0xff000000;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR), tmp_val)) return false;
		tmp_val = value & 0x00ff0000;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 1, tmp_val)) return false;
		tmp_val = value & 0x0000ff00;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 2, tmp_val)) return false;
		tmp_val = value & 0x000000ff;
		if (!io_system_cntl_reg_write((address - C2503_IO_SYS_CONTROL_BASE_ADDR) + 3, tmp_val)) return false;
		return true;
	}

	return false;
}

// Memory
//////////////////////////////////////////////////////////////////////////////////////////////
// Boot ROM
//////////////////////////////////////////////////////////////////////////////////////////////
enum flash_mode {
	FLASHMODE_READ,
	FLASHMODE_ACCESS1,
	FLASHMODE_ACCESS2,
	FLASHMODE_ACCESS3,
	FLASHMODE_ACCESS4,
	FLASHMODE_ACCESS5,
	FLASHMODE_AUTOSELECT,
	FLASHMODE_PROGRAM,
	FLASHMODE_WRITING,
};

unsigned char g_bootrom1[C2503_BOOTROM_SIZE/2];
enum flash_mode g_bootrom1_mode = FLASHMODE_READ;
unsigned char g_bootrom2[C2503_BOOTROM_SIZE/2];
enum flash_mode g_bootrom2_mode = FLASHMODE_READ;

// Initialise boot flash ROM with contents from file
bool mem_bootrom_init(FILE *fhandle) {
	unsigned char buffer, toggle = 0;
	unsigned int rom_addr = 0;

	while (fread(&buffer, 1, 1, fhandle)) {
		if (toggle) {
			g_bootrom1[rom_addr] = buffer;
			rom_addr++;
		} else {
			g_bootrom2[rom_addr] = buffer;
		}
		toggle = ~toggle;
	}
	if (ferror(fhandle)) return false;

	g_bootrom1_mode = FLASHMODE_READ;
	g_bootrom2_mode = FLASHMODE_READ;
	return true;
}

// Initialise boot flash ROM with contents from 2 files
bool mem_bootrom_split_init(FILE *fhandle1, FILE *fhandle2) {
	unsigned char tmp_store1[C2503_BOOTROM_SIZE/2], tmp_store2[C2503_BOOTROM_SIZE/2], tmp_byte;
	unsigned int rom_ptr, tmp_size_1 = 0, tmp_size_2 = 0;

	if ((tmp_size_1 = fread(tmp_store1, 1, C2503_BOOTROM_SIZE, fhandle1)) <= 0) return false;
	if ((tmp_size_2 = fread(tmp_store2, 1, C2503_BOOTROM_SIZE, fhandle2)) <= 0) return false;
	if (tmp_size_1 != tmp_size_2) return false;

	for (rom_ptr = 0; rom_ptr < tmp_size_1; rom_ptr++) {
		tmp_byte = 0;						/* Zero byte */
		// Reverse byte FW2
		if (tmp_store2[rom_ptr] & 0x01) tmp_byte |= 0x80;
		if (tmp_store2[rom_ptr] & 0x02) tmp_byte |= 0x40;
		if (tmp_store2[rom_ptr] & 0x04) tmp_byte |= 0x20;
		if (tmp_store2[rom_ptr] & 0x08) tmp_byte |= 0x10;
		if (tmp_store2[rom_ptr] & 0x10) tmp_byte |= 0x08;
		if (tmp_store2[rom_ptr] & 0x20) tmp_byte |= 0x04;
		if (tmp_store2[rom_ptr] & 0x40) tmp_byte |= 0x02;
		if (tmp_store2[rom_ptr] & 0x80) tmp_byte |= 0x01;
		g_bootrom2[rom_ptr] = tmp_byte;

		tmp_byte = 0;						/* Zero byte */
		// Reverse byte FW1
		if (tmp_store1[rom_ptr] & 0x01) tmp_byte |= 0x80;
		if (tmp_store1[rom_ptr] & 0x02) tmp_byte |= 0x40;
		if (tmp_store1[rom_ptr] & 0x04) tmp_byte |= 0x20;
		if (tmp_store1[rom_ptr] & 0x08) tmp_byte |= 0x10;
		if (tmp_store1[rom_ptr] & 0x10) tmp_byte |= 0x08;
		if (tmp_store1[rom_ptr] & 0x20) tmp_byte |= 0x04;
		if (tmp_store1[rom_ptr] & 0x40) tmp_byte |= 0x02;
		if (tmp_store1[rom_ptr] & 0x80) tmp_byte |= 0x01;
		g_bootrom1[rom_ptr] = tmp_byte;
	}

	g_bootrom1_mode = FLASHMODE_READ;
	g_bootrom2_mode = FLASHMODE_READ;
	return true;
}

// Simulate sector/chip erase
void mem_bootrom_erase1(unsigned int sector, bool all) {
	unsigned int index, size;

	sector &= 0xF0000;
	if (all) {
		index = 0;
		size = C2503_BOOTROM_SIZE/2;
	} else {
		index = sector;
		size = 0x10000;
	}
	for (; index < size; index++) {
		g_bootrom1[index] = 0;
	}
}

// Simulate sector/chip erase
void mem_bootrom_erase2(unsigned int sector, bool all) {
	unsigned int index, size;

	sector &= 0xF0000;
	if (all) {
		index = 0;
		size = C2503_BOOTROM_SIZE/2;
	} else {
		index = sector;
		size = 0x10000;
	}
	for (; index < size; index++) {
		g_bootrom2[index] = 0;
	}
}

unsigned char mem_bootrom_read1_data(unsigned int address) {
	switch (g_bootrom1_mode) {
	case FLASHMODE_AUTOSELECT:
		// Manufacturer ID
		if (address == 0x0) {
			return 0x80;
		}
		// Device ID
		if (address == 0x1) {
			return 0x25;
		}
		// Sector unprotected
		return 0x00;
		break;
	case FLASHMODE_WRITING:
		// Reset to read mode, return fake write operation status
		g_bootrom1_mode = FLASHMODE_READ;
		return 0x40;
		break;
	case FLASHMODE_READ:
	default:
		// If not in a valid mode, reset mode, and read array data
		g_bootrom1_mode = FLASHMODE_READ;
		return g_bootrom1[address];
		break;
	}
}

unsigned char mem_bootrom_read2_data(unsigned int address) {
	switch (g_bootrom2_mode) {
	case FLASHMODE_AUTOSELECT:
		// Manufacturer ID
		if (address == 0x0) {
			return 0x80;
		}
		// Device ID
		if (address == 0x1) {
			return 0x25;
		}
		// Sector unprotected
		return 0x00;
		break;
	case FLASHMODE_WRITING:
		// Reset to read mode, return fake write operation status
		g_bootrom2_mode = FLASHMODE_READ;
		return 0x40;
		break;
	case FLASHMODE_READ:
	default:
		// If not in a valid mode, reset mode, and read array data
		g_bootrom2_mode = FLASHMODE_READ;
		return g_bootrom2[address];
		break;
	}
}

bool mem_bootrom_read_byte(unsigned int address, unsigned int *value) {
	unsigned int rom_addr;
	unsigned char chip_select = 0;
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE)) && g_io_sysctrl01_0_remap_rom) {
		rom_addr = (address - C2503_BOOTROM_ADDR1) / 2;
		chip_select = 1;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		rom_addr = (address - C2503_BOOTROM_ADDR2) / 2;
		chip_select = 1;
	}
	if (chip_select) {
		if (address & 0x1) {
			// Odd addresses
			*value = mem_bootrom_read1_data(rom_addr);
		} else {
			// Even addresses
			*value = mem_bootrom_read2_data(rom_addr);
		}
		return true;
	}
	return false;
}

bool mem_bootrom_read_word(unsigned int address, unsigned int *value) {
	unsigned int rom_addr;
	unsigned char chip_select = 0;
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE)) && g_io_sysctrl01_0_remap_rom) {
		rom_addr = (address - C2503_BOOTROM_ADDR1) / 2;
		chip_select = 1;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		rom_addr = (address - C2503_BOOTROM_ADDR2) / 2;
		chip_select = 1;
	}
	if (chip_select) {
		if (address & 0x1) {
			// Odd addresses
			*value = (mem_bootrom_read1_data(rom_addr) << 8) | mem_bootrom_read2_data(rom_addr+1);
		} else {
			// Even addresses
			*value = (mem_bootrom_read2_data(rom_addr) << 8) | mem_bootrom_read1_data(rom_addr);
		}
		return true;
	}
	return false;
}

bool mem_bootrom_read_long(unsigned int address, unsigned int *value) {
	unsigned int rom_addr;
	unsigned char chip_select = 0;
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE)) && g_io_sysctrl01_0_remap_rom) {
		rom_addr = (address - C2503_BOOTROM_ADDR1) / 2;
		chip_select = 1;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		rom_addr = (address - C2503_BOOTROM_ADDR2) / 2;
		chip_select = 1;
	}
	if (chip_select) {
		if (address & 0x1) {
			// Odd addresses
			*value = (mem_bootrom_read1_data(rom_addr) << 24) | (mem_bootrom_read2_data(rom_addr+1) << 16) | \
					(mem_bootrom_read1_data(rom_addr+1) << 8) | mem_bootrom_read2_data(rom_addr+2);

		} else {
			// Even addresses
			*value = (mem_bootrom_read2_data(rom_addr) << 24) | (mem_bootrom_read1_data(rom_addr) << 16) | \
					(mem_bootrom_read2_data(rom_addr+1) << 8) | mem_bootrom_read1_data(rom_addr+1);
		}
		return true;
	}
	return false;
}

void mem_bootrom_write1_cmd(unsigned int address, unsigned char cmd) {
	// Command values are reversed (as the bus is reversed)
	switch (g_bootrom1_mode) {
	case FLASHMODE_READ:
		if ((address == 0x555) && (cmd == 0x55)) {
			g_bootrom1_mode = FLASHMODE_ACCESS1;
			return;
		}
		break;
	case FLASHMODE_ACCESS1:
		if ((address == 0x2AA) && (cmd == 0xAA)) {
			g_bootrom1_mode = FLASHMODE_ACCESS2;
			return;
		}
		break;
	case FLASHMODE_ACCESS2:
		if (address == 0x555) {
			switch (cmd) {
			case 0x01:
				g_bootrom1_mode = FLASHMODE_ACCESS3;
				return;
				break;
			case 0x05:
				g_bootrom1_mode = FLASHMODE_PROGRAM;
				return;
				break;
			case 0x09:
				g_bootrom1_mode = FLASHMODE_AUTOSELECT;
				return;
				break;
			}
		}
		break;
	case FLASHMODE_ACCESS3:
		if ((address == 0x555) && (cmd == 0x55)) {
			g_bootrom1_mode = FLASHMODE_ACCESS4;
			return;
		}
		break;
	case FLASHMODE_ACCESS4:
		if ((address == 0x2AA) && (cmd == 0xAA)) {
			g_bootrom1_mode = FLASHMODE_ACCESS5;
			return;
		}
		break;
	case FLASHMODE_ACCESS5:
		// Chip erase
		if ((address == 0x555) && (cmd == 0x08)) {
			mem_bootrom_erase1(0x0, true);
			g_bootrom1_mode = FLASHMODE_WRITING;
			return;
		}
		// Sector erase
		if (cmd == 0x0C) {
			mem_bootrom_erase1(address, false);
			g_bootrom1_mode = FLASHMODE_WRITING;
			return;
		}
		break;
	case FLASHMODE_AUTOSELECT:
		// Reset to return to read mode
		if (cmd == 0x0F) {
			g_bootrom1_mode = FLASHMODE_READ;
		}
		return;
		break;
	case FLASHMODE_PROGRAM:
		// Write byte to memory
		g_bootrom1[address] = cmd;
		g_bootrom1_mode = FLASHMODE_WRITING;
		break;
	case FLASHMODE_WRITING:
		return;
		break;
	}
	g_bootrom1_mode = FLASHMODE_READ;
}

void mem_bootrom_write2_cmd(unsigned int address, unsigned char cmd) {
	// Command values are reversed (as the bus is reversed)
	switch (g_bootrom2_mode) {
	case FLASHMODE_READ:
		if ((address == 0x555) && (cmd == 0x55)) {
			g_bootrom2_mode = FLASHMODE_ACCESS1;
			return;
		}
		break;
	case FLASHMODE_ACCESS1:
		if ((address == 0x2AA) && (cmd == 0xAA)) {
			g_bootrom2_mode = FLASHMODE_ACCESS2;
			return;
		}
		break;
	case FLASHMODE_ACCESS2:
		if (address == 0x555) {
			switch (cmd) {
			case 0x01:
				g_bootrom2_mode = FLASHMODE_ACCESS3;
				return;
				break;
			case 0x05:
				g_bootrom2_mode = FLASHMODE_PROGRAM;
				return;
				break;
			case 0x09:
				g_bootrom2_mode = FLASHMODE_AUTOSELECT;
				return;
				break;
			}
		}
		break;
	case FLASHMODE_ACCESS3:
		if ((address == 0x555) && (cmd == 0x55)) {
			g_bootrom2_mode = FLASHMODE_ACCESS4;
			return;
		}
		break;
	case FLASHMODE_ACCESS4:
		if ((address == 0x2AA) && (cmd == 0xAA)) {
			g_bootrom2_mode = FLASHMODE_ACCESS5;
			return;
		}
		break;
	case FLASHMODE_ACCESS5:
		// Chip erase
		if ((address == 0x555) && (cmd == 0x08)) {
			mem_bootrom_erase2(0x0, true);
			g_bootrom2_mode = FLASHMODE_WRITING;
			return;
		}
		// Sector erase
		if (cmd == 0x0C) {
			mem_bootrom_erase2(address, false);
			g_bootrom2_mode = FLASHMODE_WRITING;
			return;
		}
		break;
	case FLASHMODE_AUTOSELECT:
		// Reset to return to read mode
		if (cmd == 0x0F) {
			g_bootrom2_mode = FLASHMODE_READ;
		}
		return;
		break;
	case FLASHMODE_PROGRAM:
		// Write byte to memory
		g_bootrom2[address] = cmd;
		g_bootrom2_mode = FLASHMODE_WRITING;
		break;
	case FLASHMODE_WRITING:
		return;
		break;
	}
	g_bootrom2_mode = FLASHMODE_READ;
}

bool mem_bootrom_write_byte(unsigned int address, unsigned int value) {
	unsigned int rom_addr;
	unsigned char chip_select = 0;
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE)) && g_io_sysctrl01_0_remap_rom) {
		rom_addr = (address - C2503_BOOTROM_ADDR1) / 2;
		chip_select = 1;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		rom_addr = (address - C2503_BOOTROM_ADDR2) / 2;
		chip_select = 1;
	}
	if (chip_select) {
		if (address & 0x1) {
			// Odd addresses
			mem_bootrom_write1_cmd(rom_addr, (value & 0xFF));
		} else {
			// Even addresses
			mem_bootrom_write2_cmd(rom_addr, (value & 0xFF));
		}
		return true;
	}
	return false;
}

bool mem_bootrom_write_word(unsigned int address, unsigned int value) {
	unsigned int rom_addr;
	unsigned char chip_select = 0;
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE)) && g_io_sysctrl01_0_remap_rom) {
		rom_addr = (address - C2503_BOOTROM_ADDR1) / 2;
		chip_select = 1;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		rom_addr = (address - C2503_BOOTROM_ADDR2) / 2;
		chip_select = 1;
	}
	if (chip_select) {
		if (address & 0x1) {
			// Odd addresses
			mem_bootrom_write1_cmd(rom_addr, ((value & 0xFF00) >> 8));
			mem_bootrom_write2_cmd(rom_addr+1, (value & 0xFF));
		} else {
			// Even addresses
			mem_bootrom_write2_cmd(rom_addr, ((value & 0xFF00) >> 8));
			mem_bootrom_write1_cmd(rom_addr, (value & 0xFF));
		}
		return true;
	}
	return false;
}

bool mem_bootrom_write_long(unsigned int address, unsigned int value) {
	unsigned int rom_addr;
	unsigned char chip_select = 0;
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE)) && g_io_sysctrl01_0_remap_rom) {
		rom_addr = (address - C2503_BOOTROM_ADDR1) / 2;
		chip_select = 1;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		rom_addr = (address - C2503_BOOTROM_ADDR2) / 2;
		chip_select = 1;
	}
	if (chip_select) {
		if (address & 0x1) {
			// Odd addresses
			mem_bootrom_write1_cmd(rom_addr, ((value & 0xFF000000) >> 24));
			mem_bootrom_write2_cmd(rom_addr+1, ((value & 0xFF0000) >> 16));
			mem_bootrom_write1_cmd(rom_addr+1, ((value & 0xFF00) >> 8));
			mem_bootrom_write2_cmd(rom_addr+2, (value & 0xFF));
		} else {
			// Even addresses
			mem_bootrom_write2_cmd(rom_addr, ((value & 0xFF000000) >> 24));
			mem_bootrom_write1_cmd(rom_addr, ((value & 0xFF0000) >> 16));
			mem_bootrom_write2_cmd(rom_addr+1, ((value & 0xFF00) >> 8));
			mem_bootrom_write1_cmd(rom_addr+1, (value & 0xFF));
		}
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Flash ROM
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_flashrom[C2503_FLASHROM_SIZE];

// Initialise flash ROM with contents from file
bool mem_flashrom_init(FILE *fhandle) {
	unsigned int i;
	// Fake erased sectors
	for (i = 0; i < C2503_FLASHROM_SIZE; i++) {
		g_flashrom[i] = 0xFF;
	}

	if (fread(g_flashrom, 1, C2503_FLASHROM_SIZE, fhandle) <= 0) return false;
	return true;
}

bool mem_flashrom_read_byte(unsigned int address, unsigned int *value) {
	if ((address >= C2503_FLASHROM_ADDR) && (address < (C2503_FLASHROM_ADDR + C2503_FLASHROM_SIZE))) {
		*value = READ_BYTE(g_flashrom, address - C2503_FLASHROM_ADDR);
		return true;
	}
	return false;
}

bool mem_flashrom_read_word(unsigned int address, unsigned int *value) {
	if ((address >= C2503_FLASHROM_ADDR) && (address < (C2503_FLASHROM_ADDR + C2503_FLASHROM_SIZE))) {
		*value = READ_WORD(g_flashrom, address - C2503_FLASHROM_ADDR);
		return true;
	}
	return false;
}

bool mem_flashrom_read_long(unsigned int address, unsigned int *value) {
	if ((address >= C2503_FLASHROM_ADDR) && (address < (C2503_FLASHROM_ADDR + C2503_FLASHROM_SIZE))) {
		*value = READ_LONG(g_flashrom, address - C2503_FLASHROM_ADDR);
		return true;
	}
	return false;
}

bool mem_flashrom_write_byte(unsigned int address, unsigned int value) {
	if ((address >= C2503_FLASHROM_ADDR) && (address < (C2503_FLASHROM_ADDR + C2503_FLASHROM_SIZE))) {
		return true;
	}
	return false;
}

bool mem_flashrom_write_word(unsigned int address, unsigned int value) {
	if ((address >= C2503_FLASHROM_ADDR) && (address < (C2503_FLASHROM_ADDR + C2503_FLASHROM_SIZE))) {
		return true;
	}
	return false;
}

bool mem_flashrom_write_long(unsigned int address, unsigned int value) {
	if ((address >= C2503_FLASHROM_ADDR) && (address < (C2503_FLASHROM_ADDR + C2503_FLASHROM_SIZE))) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// NVRAM
//////////////////////////////////////////////////////////////////////////////////////////////
/* Data extracted from hardware */
unsigned char	g_nvram[C2503_NVRAM_SIZE] = { 0x13,0x15,0x21,0x02,0xF0,0xA5,0xAB,0xCD,0x00,0x01,0x46,0x04,0x0B,0x01,0x00,0x00, \
						0x00,0x14,0x00,0x0C,0xAC,0xA0,0x00,0x00,0x02,0x40,0x0A,0x21,0x0A,0x76,0x65,0x72, \
						0x73,0x69,0x6F,0x6E,0x20,0x31,0x31,0x2E,0x31,0x0A,0x73,0x65,0x72,0x76,0x69,0x63, \
						0x65,0x20,0x70,0x61,0x73,0x73,0x77,0x6F,0x72,0x64,0x2D,0x65,0x6E,0x63,0x72,0x79, \
						0x70,0x74,0x69,0x6F,0x6E,0x0A,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x20,0x75,0x64, \
						0x70,0x2D,0x73,0x6D,0x61,0x6C,0x6C,0x2D,0x73,0x65,0x72,0x76,0x65,0x72,0x73,0x0A, \
						0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x20,0x74,0x63,0x70,0x2D,0x73,0x6D,0x61,0x6C, \
						0x6C,0x2D,0x73,0x65,0x72,0x76,0x65,0x72,0x73,0x0A,0x21,0x0A,0x68,0x6F,0x73,0x74, \
						0x6E,0x61,0x6D,0x65,0x20,0x31,0x39,0x32,0x31,0x36,0x38,0x74,0x6F,0x70,0x0A,0x21, \
						0x0A,0x65,0x6E,0x61,0x62,0x6C,0x65,0x20,0x73,0x65,0x63,0x72,0x65,0x74,0x20,0x35, \
						0x20,0x24,0x31,0x24,0x53,0x75,0x62,0x39,0x24,0x30,0x44,0x7A,0x6B,0x2F,0x55,0x76, \
						0x30,0x55,0x56,0x38,0x4D,0x65,0x36,0x66,0x73,0x6D,0x5A,0x52,0x52,0x53,0x2F,0x0A, \
						0x21,0x0A,0x21,0x0A,0x69,0x6E,0x74,0x65,0x72,0x66,0x61,0x63,0x65,0x20,0x45,0x74, \
						0x68,0x65,0x72,0x6E,0x65,0x74,0x30,0x0A,0x20,0x69,0x70,0x20,0x61,0x64,0x64,0x72, \
						0x65,0x73,0x73,0x20,0x31,0x39,0x32,0x2E,0x31,0x36,0x38,0x2E,0x31,0x2E,0x31,0x20, \
						0x32,0x35,0x35,0x2E,0x32,0x35,0x35,0x2E,0x32,0x35,0x35,0x2E,0x30,0x0A,0x21,0x0A, \
						0x69,0x6E,0x74,0x65,0x72,0x66,0x61,0x63,0x65,0x20,0x53,0x65,0x72,0x69,0x61,0x6C, \
						0x30,0x0A,0x20,0x69,0x70,0x20,0x61,0x64,0x64,0x72,0x65,0x73,0x73,0x20,0x31,0x30, \
						0x2E,0x36,0x38,0x2E,0x30,0x2E,0x32,0x20,0x32,0x35,0x35,0x2E,0x32,0x35,0x35,0x2E, \
						0x32,0x35,0x35,0x2E,0x32,0x35,0x32,0x0A,0x20,0x63,0x6C,0x6F,0x63,0x6B,0x72,0x61, \
						0x74,0x65,0x20,0x31,0x32,0x35,0x30,0x30,0x30,0x0A,0x21,0x0A,0x69,0x6E,0x74,0x65, \
						0x72,0x66,0x61,0x63,0x65,0x20,0x53,0x65,0x72,0x69,0x61,0x6C,0x31,0x0A,0x20,0x6E, \
						0x6F,0x20,0x69,0x70,0x20,0x61,0x64,0x64,0x72,0x65,0x73,0x73,0x0A,0x20,0x73,0x68, \
						0x75,0x74,0x64,0x6F,0x77,0x6E,0x0A,0x21,0x0A,0x69,0x6E,0x74,0x65,0x72,0x66,0x61, \
						0x63,0x65,0x20,0x42,0x52,0x49,0x30,0x0A,0x20,0x6E,0x6F,0x20,0x69,0x70,0x20,0x61, \
						0x64,0x64,0x72,0x65,0x73,0x73,0x0A,0x20,0x73,0x68,0x75,0x74,0x64,0x6F,0x77,0x6E, \
						0x0A,0x21,0x0A,0x72,0x6F,0x75,0x74,0x65,0x72,0x20,0x6F,0x73,0x70,0x66,0x20,0x31, \
						0x0A,0x20,0x6E,0x65,0x74,0x77,0x6F,0x72,0x6B,0x20,0x31,0x30,0x2E,0x36,0x38,0x2E, \
						0x30,0x2E,0x30,0x20,0x30,0x2E,0x30,0x2E,0x30,0x2E,0x33,0x20,0x61,0x72,0x65,0x61, \
						0x20,0x30,0x0A,0x20,0x6E,0x65,0x74,0x77,0x6F,0x72,0x6B,0x20,0x31,0x39,0x32,0x2E, \
						0x31,0x36,0x38,0x2E,0x31,0x2E,0x30,0x20,0x30,0x2E,0x30,0x2E,0x30,0x2E,0x32,0x35, \
						0x35,0x20,0x61,0x72,0x65,0x61,0x20,0x30,0x0A,0x21,0x0A,0x6E,0x6F,0x20,0x69,0x70, \
						0x20,0x63,0x6C,0x61,0x73,0x73,0x6C,0x65,0x73,0x73,0x0A,0x21,0x0A,0x6C,0x69,0x6E, \
						0x65,0x20,0x63,0x6F,0x6E,0x20,0x30,0x0A,0x6C,0x69,0x6E,0x65,0x20,0x61,0x75,0x78, \
						0x20,0x30,0x0A,0x6C,0x69,0x6E,0x65,0x20,0x76,0x74,0x79,0x20,0x30,0x20,0x34,0x0A, \
						0x20,0x70,0x61,0x73,0x73,0x77,0x6F,0x72,0x64,0x20,0x37,0x20,0x30,0x36,0x30,0x43, \
						0x30,0x45,0x33,0x33,0x35,0x45,0x34,0x42,0x31,0x44,0x30,0x44,0x0A,0x20,0x6C,0x6F, \
						0x67,0x69,0x6E,0x0A,0x21,0x0A,0x65,0x6E,0x64,0x0A,0x64,0x74,0x68,0x20,0x36,0x34, \
						0x0A,0x20,0x73,0x68,0x75,0x74,0x64,0x6F,0x77,0x6E,0x0A,0x21,0x0A,0x69,0x6E,0x74, \
						0x65,0x72,0x66,0x61,0x63,0x65,0x20,0x42,0x52,0x49,0x30,0x0A,0x20,0x6E,0x6F,0x20, \
						0x69,0x70,0x20,0x61,0x64,0x64,0x72,0x65,0x73,0x73,0x0A,0x20,0x73,0x68,0x75,0x74, \
						0x64,0x6F,0x77,0x6E,0x0A,0x21,0x0A,0x72,0x6F,0x75,0x74,0x65,0x72,0x20,0x69,0x67, \
						0x72,0x70,0x20,0x31,0x30,0x39,0x0A,0x20,0x6E,0x65,0x74,0x77,0x6F,0x72,0x6B,0x20, \
						0x36,0x30,0x2E,0x30,0x2E,0x30,0x2E,0x30,0x0A,0x20,0x6E,0x65,0x74,0x77,0x6F,0x72, \
						0x6B,0x20,0x31,0x34,0x30,0x2E,0x31,0x2E,0x30,0x2E,0x30,0x0A,0x20,0x6E,0x65,0x74, \
						0x77,0x6F,0x72,0x6B,0x20,0x32,0x30,0x39,0x2E,0x36,0x2E,0x30,0x2E,0x30,0x0A,0x20, \
						0x6E,0x65,0x74,0x77,0x6F,0x72,0x6B,0x20,0x32,0x30,0x38,0x2E,0x39,0x2E,0x30,0x2E, \
						0x30,0x0A,0x20,0x6E,0x65,0x74,0x77,0x6F,0x72,0x6B,0x20,0x39,0x38,0x2E,0x30,0x2E, \
						0x30,0x2E,0x30,0x0A,0x21,0x0A,0x6E,0x6F,0x20,0x69,0x70,0x20,0x63,0x6C,0x61,0x73, \
						0x73,0x6C,0x65,0x73,0x73,0x0A,0x69,0x70,0x20,0x72,0x6F,0x75,0x74,0x65,0x20,0x39, \
						0x37,0x2E,0x30,0x2E,0x30,0x2E,0x30,0x20,0x32,0x35,0x35,0x2E,0x30,0x2E,0x30,0x2E, \
						0x30,0x20,0x36,0x30,0x2E,0x32,0x30,0x2E,0x34,0x2E,0x31,0x35,0x30,0x0A,0x21,0x0A, \
						0x73,0x6E,0x6D,0x70,0x2D,0x73,0x65,0x72,0x76,0x65,0x72,0x20,0x63,0x6F,0x6D,0x6D, \
						0x75,0x6E,0x69,0x74,0x79,0x20,0x70,0x75,0x62,0x6C,0x69,0x63,0x20,0x52,0x4F,0x0A, \
						0x21,0x0A,0x6C,0x69,0x6E,0x65,0x20,0x63,0x6F,0x6E,0x20,0x30,0x0A,0x20,0x65,0x78, \
						0x65,0x63,0x2D,0x74,0x69,0x6D,0x65,0x6F,0x75,0x74,0x20,0x30,0x20,0x30,0x0A,0x6C, \
						0x69,0x6E,0x65,0x20,0x61,0x75,0x78,0x20,0x30,0x0A,0x20,0x74,0x72,0x61,0x6E,0x73, \
						0x70,0x6F,0x72,0x74,0x20,0x69,0x6E,0x70,0x75,0x74,0x20,0x61,0x6C,0x6C,0x0A,0x6C, \
						0x69,0x6E,0x65,0x20,0x76,0x74,0x79,0x20,0x30,0x20,0x34,0x0A,0x20,0x70,0x61,0x73, \
						0x73,0x77,0x6F,0x72,0x64,0x20,0x6A,0x61,0x72,0x72,0x65,0x74,0x74,0x0A,0x20,0x6C, \
						0x6F,0x67,0x69,0x6E,0x0A,0x21,0x0A,0x65,0x6E,0x64,0x0A,0x00,0x00,0x00,0x00,0x00 };

unsigned char	g_nvram_last_write_byte = 0;
unsigned int	g_nvram_last_write_addr = 0;
unsigned int	g_nvram_last_write_completed = 0;

// Hardware Configuration
// 3rd/4th byte of NVRAM

// From: https://docstore.mik.ua/univercd/cc/td/doc/product/access/acs_fix/cisigslr/igslrhir/22756.htm
// Bit Number 	Meaning
// 00		Boot from ROM*
// 01-03 	Name of file for Netbooting*
// 06		Watchdog timer disabled*
// 07		OEM bit enabled
// 08		Break Disabled
// 10		IP broadcast with all zeros
// 11-12	Console line speed*
// 13		Boot default ROM software if network boot fails
// 14		IP broadcasts do not have net numbers
// 15		Run diagnostic tests and ignore NVM contents*

// Added valid header
void mem_nvram_init() {
	g_nvram[2] |= 0x80;		// Print debug messages
//	g_nvram[3] &= 0xf0;		// Boot monitor

	// Reset write status
	g_nvram_last_write_byte = 0;
	g_nvram_last_write_addr = 0;
	g_nvram_last_write_completed = 0;
}

// This isn't actually clocked, but needs to update state
void mem_nvram_core_tick() {
	if (g_nvram_last_write_completed > 0) {
		g_nvram_last_write_completed--;
		if (g_nvram_last_write_completed == 0) {
			g_nvram[g_nvram_last_write_addr] = g_nvram_last_write_byte;
		}
	}
}

// Update last write data
void mem_nvram_update_last_write(unsigned int address, unsigned char value) {
	// Complete any existing byte
	if (g_nvram_last_write_completed > 0) {
		g_nvram[g_nvram_last_write_addr] = g_nvram_last_write_byte;
	}
	// Save new data
	g_nvram_last_write_addr = address - C2503_NVRAM_ADDR;
	g_nvram_last_write_byte = value;
	// Store inverted value in NVRAM
	g_nvram[g_nvram_last_write_addr] = ~g_nvram_last_write_byte;
	// Set time to completion
	g_nvram_last_write_completed = 600;
}

bool mem_nvram_read_byte(unsigned int address, unsigned int *value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		*value = READ_BYTE(g_nvram, address - C2503_NVRAM_ADDR);
		return true;
	}
	return false;
}

bool mem_nvram_read_word(unsigned int address, unsigned int *value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		*value = READ_WORD(g_nvram, address - C2503_NVRAM_ADDR);
		return true;
	}
	return false;
}

bool mem_nvram_read_long(unsigned int address, unsigned int *value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		*value = READ_LONG(g_nvram, address - C2503_NVRAM_ADDR);
		return true;
	}
	return false;
}

bool mem_nvram_write_byte(unsigned int address, unsigned int value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		// Don't bother writing data, it'll just get overwritten
		//WRITE_BYTE(g_nvram, address - C2503_NVRAM_ADDR, value);
		mem_nvram_update_last_write(address, (value & 0xff));
		return true;
	}
	return false;
}

bool mem_nvram_write_word(unsigned int address, unsigned int value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		WRITE_WORD(g_nvram, address - C2503_NVRAM_ADDR, value);
		mem_nvram_update_last_write(address + 1, (value & 0xff));
		return true;
	}
	return false;
}

bool mem_nvram_write_long(unsigned int address, unsigned int value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		WRITE_LONG(g_nvram, address - C2503_NVRAM_ADDR, value);
		mem_nvram_update_last_write(address + 3, (value & 0xff));
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// RAM
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_ram[C2503_RAM_SIZE];

// Returns the configured (if configured) RAM window size
unsigned int mem_ram_win_size() {
	if ((g_io_sysctrl03_4_ram_ws0) && (g_io_sysctrl03_5_ram_ws1)) {
		// RAM window: 8MB
		return 0x00800000;
	} else if ((g_io_sysctrl03_4_ram_ws0) || (g_io_sysctrl03_5_ram_ws1)) {
		if (g_io_sysctrl03_4_ram_ws0) {
			// RAM window: 4MB
			return 0x00400000;
		} else {
			// RAM window: 2MB
			return 0x00200000;
		}
	} else {
		// RAM window: 16MB
		return 0x01000000;
	}
}

bool mem_ram_read_byte(unsigned int address, unsigned int *value) {
	// Handle maximum memory size
	if ((address >= C2503_RAM_ADDR) && (address < (C2503_RAM_ADDR + C2503_RAM_WIN_SIZE)) && !g_io_sysctrl01_0_remap_rom) {
		// If configured, check if outside RAM window
		if ((address - C2503_RAM_ADDR) >= mem_ram_win_size()) return false;

		// Addresses greater than RAM size are wrapped
		// RAM size must be a power of 2
		address = address & (C2503_RAM_SIZE - 1);
		*value = READ_BYTE(g_ram, address - C2503_RAM_ADDR);
		return true;
	}
	return false;
}

bool mem_ram_read_word(unsigned int address, unsigned int *value) {
	// Handle maximum memory size
	if ((address >= C2503_RAM_ADDR) && (address < (C2503_RAM_ADDR + C2503_RAM_WIN_SIZE)) && !g_io_sysctrl01_0_remap_rom) {
		// If configured, check if outside RAM window
		if ((address - C2503_RAM_ADDR) >= mem_ram_win_size()) return false;

		// Addresses greater than RAM size are wrapped
		// RAM size must be a power of 2
		address = address & (C2503_RAM_SIZE - 1);
		*value = READ_WORD(g_ram, address - C2503_RAM_ADDR);
		return true;
	}
	return false;
}

bool mem_ram_read_long(unsigned int address, unsigned int *value) {
	// Handle maximum memory size
	if ((address >= C2503_RAM_ADDR) && (address < (C2503_RAM_ADDR + C2503_RAM_WIN_SIZE)) && !g_io_sysctrl01_0_remap_rom) {
		// If configured, check if outside RAM window
		if ((address - C2503_RAM_ADDR) >= mem_ram_win_size()) return false;

		// Addresses greater than RAM size are wrapped
		// RAM size must be a power of 2
		address = address & (C2503_RAM_SIZE - 1);
		*value = READ_LONG(g_ram, address - C2503_RAM_ADDR);
		return true;
	}
	return false;
}

bool mem_ram_write_byte(unsigned int address, unsigned int value) {
	// Handle maximum memory size
	if ((address >= C2503_RAM_ADDR) && (address < (C2503_RAM_ADDR + C2503_RAM_WIN_SIZE)) && !g_io_sysctrl01_0_remap_rom) {
		// If configured, check if outside RAM window
		if ((address - C2503_RAM_ADDR) >= mem_ram_win_size()) return false;

		// Addresses greater than RAM size are wrapped
		// RAM size must be a power of 2
		address = address & (C2503_RAM_SIZE - 1);
		WRITE_BYTE(g_ram, address - C2503_RAM_ADDR, value);
		return true;
	}
	return false;
}

bool mem_ram_write_word(unsigned int address, unsigned int value) {
	// Handle maximum memory size
	if ((address >= C2503_RAM_ADDR) && (address < (C2503_RAM_ADDR + C2503_RAM_WIN_SIZE)) && !g_io_sysctrl01_0_remap_rom) {
		// If configured, check if outside RAM window
		if ((address - C2503_RAM_ADDR) >= mem_ram_win_size()) return false;

		// Addresses greater than RAM size are wrapped
		// RAM size must be a power of 2
		address = address & (C2503_RAM_SIZE - 1);
		WRITE_WORD(g_ram, address - C2503_RAM_ADDR, value);
		return true;
	}
	return false;
}

bool mem_ram_write_long(unsigned int address, unsigned int value) {
	// Handle maximum memory size
	if ((address >= C2503_RAM_ADDR) && (address < (C2503_RAM_ADDR + C2503_RAM_WIN_SIZE)) && !g_io_sysctrl01_0_remap_rom) {
		// If configured, check if outside RAM window
		if ((address - C2503_RAM_ADDR) >= mem_ram_win_size()) return false;

		// Addresses greater than RAM size are wrapped
		// RAM size must be a power of 2
		address = address & (C2503_RAM_SIZE - 1);
		WRITE_LONG(g_ram, address - C2503_RAM_ADDR, value);
		return true;
	}
	return false;
}

// I/O
//////////////////////////////////////////////////////////////////////////////////////////////
// 68302
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned int	g_io_68302_reg_bar, \
		g_io_68302_reg_scr, \
		g_io_68302_reg_ckcr;

int		g_io_68302_reg_bar_offset;

unsigned char	g_io_68302_ram1[C2503_IO_68302_RAM1_SIZE];
unsigned char	g_io_68302_ram2[C2503_IO_68302_RAM2_SIZE];
unsigned char	g_io_68302_iregs[C2503_IO_68302_REGS_SIZE];

unsigned short	g_io_68302_gimr,
		g_io_68302_ipr,
		g_io_68302_imr,
		g_io_68302_isr;

void io_68302_core_init() {
	g_io_68302_reg_bar_offset = -1;
	g_io_68302_reg_bar = 0x0000bfff;
	g_io_68302_reg_scr = 0x00000f00;
	g_io_68302_reg_ckcr = 0x00000000;

	g_io_68302_gimr = 0x0;
	g_io_68302_ipr = 0x0;
	g_io_68302_imr = 0x0;
	g_io_68302_isr = 0x0;
}

void io_68302_core_clock_tick() {
	// Clear SCP register
	g_io_68302_ram2[0x27a] = 0x0;
	g_io_68302_ram2[0x27b] = 0x0;

	// Fake values from SCC modules
	g_io_68302_ram2[0x200] = 0x5B;			// Parameter RAM [SCC3]: RxBD0
	g_io_68302_ram2[0x208] = 0xFF;			// Parameter RAM [SCC3]: RxBD1
	g_io_68302_ram2[0x000] = 0xDF;			// Parameter RAM [SCC1]: RxBD0
	g_io_68302_ram2[0x008] = 0xFB;			// Parameter RAM [SCC1]: RxBD1
	g_io_68302_ram2[0x010] = 0xF7;			// Parameter RAM [SCC1]: RxBD2
	g_io_68302_ram2[0x018] = 0xEA;			// Parameter RAM [SCC1]: RxBD3
	g_io_68302_ram2[0x020] = 0xEF;			// Parameter RAM [SCC1]: RxBD4
	g_io_68302_ram2[0x028] = 0xFF;			// Parameter RAM [SCC1]: RxBD5
	g_io_68302_ram2[0x030] = 0xDF;			// Parameter RAM [SCC1]: RxBD6
	g_io_68302_ram2[0x038] = 0xFE;			// Parameter RAM [SCC1]: RxBD7
	g_io_68302_ram2[0x100] = 0x77;			// Parameter RAM [SCC2]: RxBD0
	g_io_68302_ram2[0x108] = 0xDD;			// Parameter RAM [SCC2]: RxBD1
	g_io_68302_ram2[0x110] = 0xBB;			// Parameter RAM [SCC2]: RxBD2
	g_io_68302_ram2[0x118] = 0xDF;			// Parameter RAM [SCC2]: RxBD3
	g_io_68302_ram2[0x120] = 0x9F;			// Parameter RAM [SCC2]: RxBD4
	g_io_68302_ram2[0x128] = 0xFB;			// Parameter RAM [SCC2]: RxBD5
	g_io_68302_ram2[0x130] = 0xEF;			// Parameter RAM [SCC2]: RxBD6
	g_io_68302_ram2[0x138] = 0xBC;			// Parameter RAM [SCC2]: RxBD7
}

bool io_68302_isIRQ() {
	if (g_io_68302_ipr & g_io_68302_imr)
		return true;
	else
		return false;
}

bool io_68302_read_byte(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_3_reset_68302) return false;		// If device is currently reset, access is not allowed

	// 68302 BAR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR)) && \
				(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR + 4))) {
		switch (address & 0x3) {
			case 0:
				*value = (g_io_68302_reg_bar & 0xff000000) >> 24;
				break;
			case 1:
				*value = (g_io_68302_reg_bar & 0x00ff0000) >> 16;
				break;
			case 2:
				*value = (g_io_68302_reg_bar & 0x0000ff00) >> 8;
				break;
			case 3:
				*value = g_io_68302_reg_bar & 0x000000ff;
				break;
		}
		return true;
	}
	// 68302 CSR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR)) && \
				(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR + 4))) {
		switch (address & 0x3) {
			case 0:
				*value = (g_io_68302_reg_scr & 0xff000000) >> 24;
				break;
			case 1:
				*value = (g_io_68302_reg_scr & 0x00ff0000) >> 16;
				break;
			case 2:
				*value = (g_io_68302_reg_scr & 0x0000ff00) >> 8;
				break;
			case 3:
				*value = g_io_68302_reg_scr & 0x000000ff;
				break;
		}
		return true;
	}
	// 68302 CKCR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR)) && \
				(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR + 4))) {
		switch (address & 0x3) {
			case 0:
				*value = (g_io_68302_reg_ckcr & 0xff000000) >> 24;
				break;
			case 1:
				*value = (g_io_68302_reg_ckcr & 0x00ff0000) >> 16;
				break;
			case 2:
				*value = (g_io_68302_reg_ckcr & 0x0000ff00) >> 8;
				break;
			case 3:
				*value = g_io_68302_reg_ckcr & 0x000000ff;
				break;
		}
		return true;
	}
	if (g_io_68302_reg_bar_offset >= 0) {
		// System RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR + C2503_IO_68302_RAM1_SIZE))) {
			*value = READ_BYTE(g_io_68302_ram1, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR));
			return true;
		}
		// Parameter RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR + C2503_IO_68302_RAM2_SIZE))) {
			*value = READ_BYTE(g_io_68302_ram2, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR));
			return true;
		}
		// Internal Registers
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR + C2503_IO_68302_REGS_SIZE))) {
			*value = READ_BYTE(g_io_68302_iregs, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR));
			return true;
		}
	}
	return false;
}

bool io_68302_read_word(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_3_reset_68302) return false;		// If device is currently reset, access is not allowed

	// 68302 BAR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR)) && \
			(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR + 4))) {
		if (address & 0x2) {				// Upper or Lower?
			*value = g_io_68302_reg_bar & 0x0000ffff;
		} else {
			*value = (g_io_68302_reg_bar & 0xffff0000) >> 16;
		}
		return true;
	}
	// 68302 SCR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR)) && \
			(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR + 4))) {
		if (address & 0x2) {				// Upper or Lower?
			*value = g_io_68302_reg_scr & 0x0000ffff;
		} else {
			*value = (g_io_68302_reg_scr & 0xffff0000) >> 16;
		}
		return true;
	}
	// 68302 CKCR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR)) && \
			(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR + 4))) {
		if (address & 0x2) {				// Upper or Lower?
			*value = g_io_68302_reg_ckcr & 0x0000ffff;
		} else {
			*value = (g_io_68302_reg_ckcr & 0xffff0000) >> 16;
		}
		return true;
	}
	if (g_io_68302_reg_bar_offset >= 0) {
		// System RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR + C2503_IO_68302_RAM1_SIZE))) {
			*value = READ_WORD(g_io_68302_ram1, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR));
			return true;
		}
		// Parameter RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR + C2503_IO_68302_RAM2_SIZE))) {
			*value = READ_WORD(g_io_68302_ram2, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR));
			return true;
		}
		// Internal Registers
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR + C2503_IO_68302_REGS_SIZE))) {
			*value = READ_WORD(g_io_68302_iregs, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR));
			return true;
		}
	}
	return false;
}

bool io_68302_read_long(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_3_reset_68302) return false;		// If device is currently reset, access is not allowed

	// 68302 BAR register
	if (address == (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR)) {
		*value = g_io_68302_reg_bar;
		return true;
	}
	// 68302 SCR register
	if (address == (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR)) {
		*value = g_io_68302_reg_scr;
		return true;
	}
	// 68302 CKCR register
	if (address == (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR)) {
		*value = g_io_68302_reg_ckcr;
		return true;
	}
	if (g_io_68302_reg_bar_offset >= 0) {
		// System RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR + C2503_IO_68302_RAM1_SIZE))) {
			*value = READ_LONG(g_io_68302_ram1, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR));
			return true;
		}
		// Parameter RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR + C2503_IO_68302_RAM2_SIZE))) {
			*value = READ_LONG(g_io_68302_ram2, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR));
			return true;
		}
		// Internal Registers
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR + C2503_IO_68302_REGS_SIZE))) {
			*value = READ_LONG(g_io_68302_iregs, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR));
			return true;
		}
	}
	return false;
}

bool io_68302_write_byte(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_3_reset_68302) return false;		// If device is currently reset, access is not allowed

	// 68302 BAR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR)) && \
				(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR + 4))) {
		switch (address & 0x3) {
			case 0:
				g_io_68302_reg_bar = (g_io_68302_reg_bar & 0x00ffffff) | ((value & 0xff) << 24);
				break;
			case 1:
				g_io_68302_reg_bar = (g_io_68302_reg_bar & 0xff00ffff) | ((value & 0xff) << 16);
				break;
			case 2:
				g_io_68302_reg_bar = (g_io_68302_reg_bar & 0xffff00ff) | ((value & 0xff) << 8);
				break;
			case 3:
				g_io_68302_reg_bar = (g_io_68302_reg_bar & 0xffffff00) | (value & 0xff);
				g_io_68302_reg_bar_offset = (g_io_68302_reg_bar & 0x00000fff) << 12;
				break;
		}
		return true;
	}
	// 68302 CSR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR)) && \
				(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR + 4))) {
		switch (address & 0x3) {
			case 0:
				g_io_68302_reg_scr = (g_io_68302_reg_scr & 0x00ffffff) | ((value & 0xff) << 24);
				break;
			case 1:
				g_io_68302_reg_scr = (g_io_68302_reg_scr & 0xff00ffff) | ((value & 0xff) << 16);
				break;
			case 2:
				g_io_68302_reg_scr = (g_io_68302_reg_scr & 0xffff00ff) | ((value & 0xff) << 8);
				break;
			case 3:
				g_io_68302_reg_scr = (g_io_68302_reg_scr & 0xffffff00) | (value & 0xff);
				break;
		}
		return true;
	}
	// 68302 CKCR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR)) && \
				(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR + 4))) {
		switch (address & 0x3) {
			case 0:
				g_io_68302_reg_ckcr = (g_io_68302_reg_ckcr & 0x00ffffff) | ((value & 0xff) << 24);
				break;
			case 1:
				g_io_68302_reg_ckcr = (g_io_68302_reg_ckcr & 0xff00ffff) | ((value & 0xff) << 16);
				break;
			case 2:
				g_io_68302_reg_ckcr = (g_io_68302_reg_ckcr & 0xffff00ff) | ((value & 0xff) << 8);
				break;
			case 3:
				g_io_68302_reg_ckcr = (g_io_68302_reg_ckcr & 0xffffff00) | (value & 0xff);
				break;
		}
		return true;
	}
	if (g_io_68302_reg_bar_offset >= 0) {
		// System RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR + C2503_IO_68302_RAM1_SIZE))) {
			WRITE_BYTE(g_io_68302_ram1, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR), value);
			return true;
		}
		// Parameter RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR + C2503_IO_68302_RAM2_SIZE))) {
			WRITE_BYTE(g_io_68302_ram2, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR), value);
			return true;
		}
		// Internal Registers
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR + C2503_IO_68302_REGS_SIZE))) {
			WRITE_BYTE(g_io_68302_iregs, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR), value);
			return true;
		}
	}
	return false;
}

bool io_68302_write_word(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_3_reset_68302) return false;		// If device is currently reset, access is not allowed

	// 68302 BAR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR)) && \
			(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR + 4))) {
		if (address & 0x2) {				// Upper or Lower?
			g_io_68302_reg_bar = (g_io_68302_reg_bar & 0xffff0000) | (value & 0x0000ffff);
			g_io_68302_reg_bar_offset = (g_io_68302_reg_bar & 0x00000fff) << 12;
		} else {
			g_io_68302_reg_bar = (g_io_68302_reg_bar & 0x0000ffff) | ((value & 0x0000ffff) << 16);
		}
		return true;
	}
	// 68302 SCR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR)) && \
			(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR + 4))) {
		if (address & 0x2) {				// Upper or Lower?
			g_io_68302_reg_scr = (g_io_68302_reg_scr & 0xffff0000) | (value & 0x0000ffff);
		} else {
			g_io_68302_reg_scr = (g_io_68302_reg_scr & 0x0000ffff) | ((value & 0x0000ffff) << 16);
		}
		return true;
	}
	// 68302 CKCR register
	if ((address >= (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR)) && \
			(address < (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR + 4))) {
		if (address & 0x2) {				// Upper or Lower?
			g_io_68302_reg_ckcr = (g_io_68302_reg_ckcr & 0xffff0000) | (value & 0x0000ffff);
		} else {
			g_io_68302_reg_ckcr = (g_io_68302_reg_ckcr & 0x0000ffff) | ((value & 0x0000ffff) << 16);
		}
		return true;
	}
	if (g_io_68302_reg_bar_offset >= 0) {
		// System RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR + C2503_IO_68302_RAM1_SIZE))) {
			WRITE_WORD(g_io_68302_ram1, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR), value);
			return true;
		}
		// Parameter RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR + C2503_IO_68302_RAM2_SIZE))) {
			WRITE_WORD(g_io_68302_ram2, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR), value);
			return true;
		}
		// Internal Registers
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR + C2503_IO_68302_REGS_SIZE))) {
			WRITE_WORD(g_io_68302_iregs, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR), value);
			return true;
		}
	}
	return false;
}

bool io_68302_write_long(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_3_reset_68302) return false;		// If device is currently reset, access is not allowed

	// 68302 BAR register
	if (address == (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_BAR_ADDR)) {
		g_io_68302_reg_bar = value;
		g_io_68302_reg_bar_offset = (g_io_68302_reg_bar & 0x00000fff) << 12;
		return true;
	}
	// 68302 SCR register
	if (address == (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_SCR_ADDR)) {
		g_io_68302_reg_scr = value;
		return true;
	}
	// 68302 CKCR register
	if (address == (C2503_IO_68302_BASE_ADDR + C2503_IO_68302_CKCR_ADDR)) {
		g_io_68302_reg_ckcr = value;
		return true;
	}
	if (g_io_68302_reg_bar_offset >= 0) {
		// System RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR + C2503_IO_68302_RAM1_SIZE))) {
			WRITE_LONG(g_io_68302_ram1, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM1_ADDR), value);
			return true;
		}
		// Parameter RAM
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR + C2503_IO_68302_RAM2_SIZE))) {
			WRITE_LONG(g_io_68302_ram2, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_RAM2_ADDR), value);
			return true;
		}
		// Internal Registers
		if ((address >= (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR)) && \
					(address < (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR + C2503_IO_68302_REGS_SIZE))) {
			WRITE_LONG(g_io_68302_iregs, address - (C2503_IO_68302_BASE_ADDR + g_io_68302_reg_bar_offset + C2503_IO_68302_REGS_ADDR), value);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Counter / Timer
//////////////////////////////////////////////////////////////////////////////////////////////
// Need to implement irq status clearing

bool		g_io_counter_running;
bool		g_io_counter_watchdog_enabled;
unsigned short	g_io_counter_reg2, \
		g_io_counter_reg3, \
		g_io_counter_count, \
		g_io_counter_initial;

void io_counter_core_init() {
		g_io_counter_running = false;
		g_io_counter_watchdog_enabled = false;
		g_io_counter_reg2 = 0;
		g_io_counter_reg3 = 0;
		g_io_counter_count = 0;
		g_io_counter_initial = 0xffff;
}

void io_counter_core_clock_tick() {
	if (g_io_counter_running) {					/* If the counter is running */
		g_io_counter_count--;					/* Decrement the count */
		if (g_io_counter_count == 0x00) {			/* If the count is zero */
			g_io_counter_count = g_io_counter_initial;	/* Reset counter */
			m68k_set_irq(7);				/* Trigger NMI */
		}
	}
}

bool io_counter_read_byte(unsigned int address, unsigned int *value) {
	// Interrupt status
	if ((address >= C2503_IO_COUNTER_REG1_ADDR) && (address < (C2503_IO_COUNTER_REG1_ADDR + C2503_IO_COUNTER_REG1_SIZE))) {
		if (g_io_counter_watchdog_enabled) *value |= 0x40;
		return true;
	}
	// Counter Register 2
	if ((address >= C2503_IO_COUNTER_REG2_ADDR) && (address < (C2503_IO_COUNTER_REG2_ADDR + C2503_IO_COUNTER_REG2_SIZE))) {
		if (address & 0x1) {
			*value = g_io_counter_reg2 & 0x00ff;
		} else {
			*value = (g_io_counter_reg2 & 0xff00) >> 8;
		}
		return true;
	}
	// Counter Register 3
	if ((address >= C2503_IO_COUNTER_REG3_ADDR) && (address < (C2503_IO_COUNTER_REG3_ADDR + C2503_IO_COUNTER_REG3_SIZE))) {
		if (address & 0x1) {
			*value = g_io_counter_reg3 & 0x00ff;
		} else {
			*value = (g_io_counter_reg3 & 0xff00) >> 8;
		}
		return true;
	}
	// Get current value of counter
	if ((address >= C2503_IO_COUNTER_REG4_ADDR) && (address < (C2503_IO_COUNTER_REG4_ADDR + C2503_IO_COUNTER_REG4_SIZE))) {
		if (address & 0x1) {
			*value = g_io_counter_count & 0x00ff;
		} else {
			*value = (g_io_counter_count & 0xff00) >> 8;
		}
		return true;
	}

	return false;
}

bool io_counter_read_word(unsigned int address, unsigned int *value) {
	// Counter Register 2
	if ((address >= C2503_IO_COUNTER_REG2_ADDR) && (address < (C2503_IO_COUNTER_REG2_ADDR + C2503_IO_COUNTER_REG2_SIZE))) {
		*value = g_io_counter_reg2;
		return true;
	}
	// Counter Register 3
	if ((address >= C2503_IO_COUNTER_REG3_ADDR) && (address < (C2503_IO_COUNTER_REG3_ADDR + C2503_IO_COUNTER_REG3_SIZE))) {
		*value = g_io_counter_reg3;
		return true;
	}
	// Get current value of counter
	if ((address >= C2503_IO_COUNTER_REG4_ADDR) && (address < (C2503_IO_COUNTER_REG4_ADDR + C2503_IO_COUNTER_REG4_SIZE))) {
		*value = g_io_counter_count;
		return true;
	}

	return false;
}

bool io_counter_write_byte(unsigned int address, unsigned int value) {
	// Counter control
	if ((address >= C2503_IO_COUNTER_REG1_ADDR) && (address < (C2503_IO_COUNTER_REG1_ADDR + C2503_IO_COUNTER_REG1_SIZE))) {
		if (value == 0x00) {
			g_io_counter_running = true;
			g_io_counter_count = g_io_counter_initial;
		}
		if (value & 0x40) {
			g_io_counter_watchdog_enabled = true;
		} else {
			g_io_counter_watchdog_enabled = false;
		}
		return true;
	}
	// Counter Register 2
	if ((address >= C2503_IO_COUNTER_REG2_ADDR) && (address < (C2503_IO_COUNTER_REG2_ADDR + C2503_IO_COUNTER_REG2_SIZE))) {
		if (address & 0x1) {
			g_io_counter_reg2 = (g_io_counter_reg2 & 0xff00) | (value & 0xff);
		} else {
			g_io_counter_reg2 = (g_io_counter_reg2 & 0x00ff) | ((value & 0xff) << 8);
		}
		return true;
	}
	// Counter Register 3
	if ((address >= C2503_IO_COUNTER_REG3_ADDR) && (address < (C2503_IO_COUNTER_REG3_ADDR + C2503_IO_COUNTER_REG3_SIZE))) {
		if (address & 0x1) {
			g_io_counter_reg3 = (g_io_counter_reg3 & 0xff00) | (value & 0xff);
		} else {
			g_io_counter_reg3 = (g_io_counter_reg3 & 0x00ff) | ((value & 0xff) << 8);
		}
		return true;
	}
	// Set initial value of counter on rollover
	if ((address >= C2503_IO_COUNTER_REG4_ADDR) && (address < (C2503_IO_COUNTER_REG4_ADDR + C2503_IO_COUNTER_REG4_SIZE))) {
		if (address & 0x1) {
			g_io_counter_initial = (g_io_counter_count & 0xff00) | (value & 0xff);
		} else {
			g_io_counter_initial = (g_io_counter_count & 0x00ff) | ((value & 0xff) << 8);
		}
		return true;
	}

	return false;
}

bool io_counter_write_word(unsigned int address, unsigned int value) {
	// Counter Register 2
	if ((address >= C2503_IO_COUNTER_REG2_ADDR) && (address < (C2503_IO_COUNTER_REG2_ADDR + C2503_IO_COUNTER_REG2_SIZE))) {
		g_io_counter_reg2 = value & 0xffff;
		return true;
	}
	// Counter Register 3
	if ((address >= C2503_IO_COUNTER_REG3_ADDR) && (address < (C2503_IO_COUNTER_REG3_ADDR + C2503_IO_COUNTER_REG3_SIZE))) {
		g_io_counter_reg3 = value & 0xffff;
		return true;
	}
	// Set initial value of counter on rollover
	if ((address >= C2503_IO_COUNTER_REG4_ADDR) && (address < (C2503_IO_COUNTER_REG4_ADDR + C2503_IO_COUNTER_REG4_SIZE))) {
		if (value == 0x1) {
			g_io_counter_running = false;
		} else {
			g_io_counter_initial = value & 0xffff;
		}
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Dual UART
//////////////////////////////////////////////////////////////////////////////////////////////
bool		\
		// Channel A
		scn2681_channelA_mode2_selected, scn2681_channelA_rx_rts_enabled, \
		scn2681_channelA_rx_enabled, scn2681_channelA_rx_rsr_empty, scn2681_channelA_rx_status_overrun, \
		scn2681_channelA_tx_enabled, scn2681_channelA_tx_thr_empty, scn2681_channelA_tx_tsr_empty, \
		// Channel B
		scn2681_channelB_mode2_selected, scn2681_channelB_rx_rts_enabled, \
		scn2681_channelB_rx_enabled, scn2681_channelB_rx_rsr_empty, scn2681_channelB_rx_status_overrun, \
		scn2681_channelB_tx_enabled, scn2681_channelB_tx_thr_empty, scn2681_channelB_tx_tsr_empty;

unsigned char	\
		// Channel A Registers
		scn2681_channelA_mode1, scn2681_channelA_mode2, scn2681_channelA_status, \
		scn2681_channelA_clock_select, scn2681_channelA_command, \
		scn2681_channelA_rx_rsr, scn2681_channelA_rx_rd_idx, scn2681_channelA_rx_wr_idx, \
		scn2681_channelA_tx_thr, scn2681_channelA_tx_tsr, \
		// Channel B Registers
		scn2681_channelB_mode1, scn2681_channelB_mode2, scn2681_channelB_status, \
		scn2681_channelB_clock_select, scn2681_channelB_command, \
		scn2681_channelB_rx_rsr, scn2681_channelB_rx_rd_idx, scn2681_channelB_rx_wr_idx, \
		scn2681_channelB_tx_thr, scn2681_channelB_tx_tsr, \
		// Input Port
		scn2681_input_port, scn2681_input_port_change, \
		// Output Port
		scn2681_output_port_configuration, scn2681_output_port_set, scn2681_output_port_reset, \
		// Interrupts
		scn2681_interrupt_mask, scn2681_interrupt_status, \
		// Misc
		scn2681_auxiliary_control, \
		// Counter / Timer
		scn2681_counter_timer_upper, scn2681_counter_timer_lower, \
		scn2681_counter_timer_upper_preset, scn2681_counter_timer_lower_preset, \
		scn2681_counter_start_command, scn2681_counter_stop_command;

unsigned int	\
		// Channel A
		scn2681_channelA_rx_fifo[SCN2681_REG_RX_FIFO_SIZE],
		scn2681_channelA_rx_rsr_ticks, \
		scn2681_channelA_tx_tsr_ticks, \
		// Channel B
		scn2681_channelB_rx_fifo[SCN2681_REG_RX_FIFO_SIZE],
		scn2681_channelB_rx_rsr_ticks, \
		scn2681_channelB_tx_tsr_ticks;

int		\
		// Channel A
		scn2681_channelA_serial_device_fd = -1, \
		// Channel B
		scn2681_channelB_serial_device_fd = -1;

// Sets a file handler to use as a serial device for channel
//////////////////////////////////////////////////////////////////////////////////////////////
void io_duart_core_channel_fd_set_RTS(int fd_device) {
	unsigned int status;

	// Check if FD valid
	if (fd_device == -1) return;

	ioctl(fd_device, TIOCMGET, &status);
	status |= TIOCM_RTS;
	ioctl(fd_device, TIOCMSET, &status);
}

void io_duart_core_channel_fd_clear_RTS(int fd_device) {
	unsigned int status;

	// Check if FD valid
	if (fd_device == -1) return;

	ioctl(fd_device, TIOCMGET, &status);
	status &= ~TIOCM_RTS;
	ioctl(fd_device, TIOCMSET, &status);
}

void io_duart_core_channel_fd_set_hwflow(int fd_device) {
	struct termios options;

	// Check if FD valid
	if (fd_device == -1) return;

	// Get current serial port configuration
	tcgetattr(fd_device, &options);

	// Enable hardware flow control
	options.c_cflag |= CRTSCTS;

	// Set current serial port configuration
	tcsetattr(fd_device, TCSANOW, &options);
}

void io_duart_core_channel_fd_set_mode1(int fd_device, unsigned char register_val) {
	struct termios options;

	// Check if FD valid
	if (fd_device == -1) return;

	// Get current serial port configuration
	tcgetattr(fd_device, &options);

	// Clear existing options
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;

	// Set symbol length
	switch (register_val & 0x03) {
		case 0:
			options.c_cflag |= CS5;
			break;
		case 1:
			options.c_cflag |= CS6;
			break;
		case 2:
			options.c_cflag |= CS7;
			break;
		case 3:
			options.c_cflag |= CS8;
			break;
	}

	// Set parity type
	if (register_val & 0x04) options.c_cflag |= PARODD;

	// Parity enabled
	if ((register_val & 0x18) < 0x10) {
		options.c_cflag |= PARENB;
	}

	// Set current serial port configuration
	tcsetattr(fd_device, TCSANOW, &options);

	// Make sure RTS signal is enabled if RTS control is disabled
	if ((register_val & SCN2681_REG_MODE1_RX_RTS) == 0) {
		io_duart_core_channel_fd_set_RTS(fd_device);
	}
}

void io_duart_core_channel_fd_set_baud(int fd_device, unsigned char baud_selection, unsigned int brg_set_select) {
	struct termios options;
	unsigned char ispeed, ospeed, *brg_set;

	unsigned char brg_set0[16] = {B50, B110, B134, B200, B300, B600, B1200, B0, B2400, B4800, B0, B9600, B38400, B0, B0, B0};
	unsigned char brg_set1[16] = {B75, B110, B134, B150, B300, B600, B1200, B0, B2400, B4800, B1800, B9600, B19200, B0, B0, B0};

	// Check if FD valid
	if (fd_device == -1) return;

	if (brg_set_select) {
		brg_set = brg_set1;
	} else {
		brg_set = brg_set0;
	}

	ispeed = brg_set[(baud_selection & 0xF0) >> 4];
	ospeed = brg_set[(baud_selection & 0x0F)];

	// Get current serial port configuration
	tcgetattr(fd_device, &options);

	// Set baud rate
	cfsetispeed(&options, ispeed);
	cfsetospeed(&options, ospeed);

	// Set current serial port configuration
	tcsetattr(fd_device, TCSANOW, &options);
}

void io_duart_core_channelA_serial_device(int fd_device) {
	scn2681_channelA_serial_device_fd = fd_device;
	io_duart_core_channel_fd_set_hwflow(fd_device);
	io_duart_core_channel_fd_set_RTS(fd_device);
	scn2681_channelA_rx_rts_enabled = true;
}

void io_duart_core_channelB_serial_device(int fd_device) {
	scn2681_channelB_serial_device_fd = fd_device;
	io_duart_core_channel_fd_set_hwflow(fd_device);
	io_duart_core_channel_fd_set_RTS(fd_device);
	scn2681_channelB_rx_rts_enabled = true;
}

// Read DUART core registers directly (for instrumentation)
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char io_duart_core_get_reg(enum scn2681_core_reg regname) {
	switch (regname) {
	// Channel A
		case ChannelA_Mode1:
			return scn2681_channelA_mode1;
			break;
		case ChannelA_Mode2:
			return scn2681_channelA_mode2;
			break;
		case ChannelA_Status:
			return scn2681_channelA_status;
			break;
		case ChannelA_Clock_Select:
			return scn2681_channelA_clock_select;
			break;
		case ChannelA_Command:
			return scn2681_channelA_command;
			break;
		case ChannelA_Rx_Fifo0:
			return 0xff & scn2681_channelA_rx_fifo[0];
			break;
		case ChannelA_Rx_Fifo0_Status:
			return 0xff & (scn2681_channelA_rx_fifo[0]>>8);
			break;
		case ChannelA_Rx_Fifo1:
			return 0xff & scn2681_channelA_rx_fifo[1];
			break;
		case ChannelA_Rx_Fifo1_Status:
			return 0xff & (scn2681_channelA_rx_fifo[1]>>8);
			break;
		case ChannelA_Rx_Fifo2:
			return 0xff & scn2681_channelA_rx_fifo[2];
			break;
		case ChannelA_Rx_Fifo2_Status:
			return 0xff & (scn2681_channelA_rx_fifo[2]>>8);
			break;
		case ChannelA_Rx_Read_Index:
			return scn2681_channelA_rx_rd_idx;
			break;
		case ChannelA_Rx_Write_Index:
			return scn2681_channelA_rx_wr_idx;
			break;
		case ChannelA_Rx_Shift:
			return scn2681_channelA_rx_rsr;
			break;
		case ChannelA_Rx_Shift_Empty:
			return scn2681_channelA_rx_rsr_empty;
			break;
		case ChannelA_Tx_Holding:
			return scn2681_channelA_tx_thr;
			break;
		case ChannelA_Tx_Holding_Empty:
			return scn2681_channelA_tx_thr_empty;
			break;
		case ChannelA_Tx_Shift:
			return scn2681_channelA_tx_tsr;
			break;
		case ChannelA_Tx_Shift_Empty:
			return scn2681_channelA_tx_tsr_empty;
			break;
	// Channel B
		case ChannelB_Mode1:
			return scn2681_channelB_mode1;
			break;
		case ChannelB_Mode2:
			return scn2681_channelB_mode2;
			break;
		case ChannelB_Status:
			return scn2681_channelB_status;
			break;
		case ChannelB_Clock_Select:
			return scn2681_channelB_clock_select;
			break;
		case ChannelB_Command:
			return scn2681_channelB_command;
			break;
		case ChannelB_Rx_Fifo0:
			return 0xff & scn2681_channelB_rx_fifo[0];
			break;
		case ChannelB_Rx_Fifo0_Status:
			return 0xff & (scn2681_channelB_rx_fifo[0]>>8);
			break;
		case ChannelB_Rx_Fifo1:
			return 0xff & scn2681_channelB_rx_fifo[1];
			break;
		case ChannelB_Rx_Fifo1_Status:
			return 0xff & (scn2681_channelB_rx_fifo[1]>>8);
			break;
		case ChannelB_Rx_Fifo2:
			return 0xff & scn2681_channelB_rx_fifo[2];
			break;
		case ChannelB_Rx_Fifo2_Status:
			return 0xff & (scn2681_channelB_rx_fifo[2]>>8);
			break;
		case ChannelB_Rx_Read_Index:
			return scn2681_channelB_rx_rd_idx;
			break;
		case ChannelB_Rx_Write_Index:
			return scn2681_channelB_rx_wr_idx;
			break;
		case ChannelB_Rx_Shift:
			return scn2681_channelB_rx_rsr;
			break;
		case ChannelB_Rx_Shift_Empty:
			return scn2681_channelB_rx_rsr_empty;
			break;
		case ChannelB_Tx_Holding:
			return scn2681_channelB_tx_thr;
			break;
		case ChannelB_Tx_Holding_Empty:
			return scn2681_channelB_tx_thr_empty;
			break;
		case ChannelB_Tx_Shift:
			return scn2681_channelB_tx_tsr;
			break;
		case ChannelB_Tx_Shift_Empty:
			return scn2681_channelB_tx_tsr_empty;
			break;
	// Interrupts
		case Interrupt_Mask:
			return scn2681_interrupt_mask;
			break;
		case Interrupt_Status:
			return scn2681_interrupt_status;
			break;
	// Misc
		case Auxiliary_Control:
			return scn2681_auxiliary_control;
			break;
	// Counter / Timer
		case CounterTimer_Upper:
			return scn2681_counter_timer_upper;
			break;
		case CounterTimer_Lower:
			return scn2681_counter_timer_lower;
			break;
		case CounterTimer_Upper_Preset:
			return scn2681_counter_timer_upper_preset;
			break;
		case CounterTimer_Lower_Preset:
			return scn2681_counter_timer_lower_preset;
			break;
		case CounterTimer_Start_Command:
			return scn2681_counter_start_command;
			break;
		case CounterTimer_Stop_Command:
			return scn2681_counter_stop_command;
			break;
	// Input Port
		case Input_Port:
			return scn2681_input_port;
			break;
		case Input_Port_Change:
			return scn2681_input_port_change;
			break;
	// Output Port
		case Output_Port_Configuration:
			return scn2681_output_port_configuration;
			break;
		case Output_Port_Set:
			return scn2681_output_port_set;
			break;
		case Output_Port_Reset:
			return scn2681_output_port_reset;
			break;
	}

	return 0;
}

// Channel A functions
//////////////////////////////////////////////////////////////////////////////////////////////
// Misc
void io_duart_core_channelA_reset_selected_mr() {
	scn2681_channelA_mode2_selected = false;
}
void io_duart_core_channelA_reset_error_status() {
	scn2681_channelA_rx_status_overrun = false;
	scn2681_channelA_status = scn2681_channelA_status & 0x0F;
}

// Rx
void io_duart_core_channelA_rx_enable() {
	scn2681_channelA_rx_enabled = true;
}
void io_duart_core_channelA_rx_disable() {
	scn2681_channelA_rx_enabled = false;
}
void io_duart_core_channelA_rx_reset() {
	io_duart_core_channelA_rx_disable();
	scn2681_channelA_rx_fifo[0] = 0;
	scn2681_channelA_rx_fifo[1] = 0;
	scn2681_channelA_rx_fifo[2] = 0;
	scn2681_channelA_rx_rd_idx = 0;
	scn2681_channelA_rx_wr_idx = 0;
	scn2681_channelA_rx_rsr = 0;
	scn2681_channelA_rx_rsr_ticks = 0;
	scn2681_channelA_rx_rsr_empty = true;
	scn2681_channelA_rx_status_overrun = false;
	io_duart_core_channel_fd_set_RTS(scn2681_channelA_serial_device_fd);
	scn2681_channelA_rx_rts_enabled = true;
}

// Tx
void io_duart_core_channelA_tx_enable() {
	scn2681_channelA_tx_enabled = true;
}
void io_duart_core_channelA_tx_disable() {
	scn2681_channelA_tx_enabled = false;
}
void io_duart_core_channelA_tx_reset() {
	io_duart_core_channelA_tx_disable();
	scn2681_channelA_tx_thr = 0;
	scn2681_channelA_tx_thr_empty = true;
	scn2681_channelA_tx_tsr = 0;
	scn2681_channelA_tx_tsr_empty = true;
	scn2681_channelA_tx_tsr_ticks = 0;
}

// Channel B functions
//////////////////////////////////////////////////////////////////////////////////////////////
// Misc
void io_duart_core_channelB_reset_selected_mr() {
	scn2681_channelB_mode2_selected = false;
}
void io_duart_core_channelB_reset_error_status() {
	scn2681_channelB_rx_status_overrun = false;
	scn2681_channelB_status = scn2681_channelB_status & 0x0F;
}

// Rx
void io_duart_core_channelB_rx_enable() {
	scn2681_channelB_rx_enabled = true;
}
void io_duart_core_channelB_rx_disable() {
	scn2681_channelB_rx_enabled = false;
}
void io_duart_core_channelB_rx_reset() {
	io_duart_core_channelB_rx_disable();
	scn2681_channelB_rx_fifo[0] = 0;
	scn2681_channelB_rx_fifo[1] = 0;
	scn2681_channelB_rx_fifo[2] = 0;
	scn2681_channelB_rx_rd_idx = 0;
	scn2681_channelB_rx_wr_idx = 0;
	scn2681_channelB_rx_rsr = 0;
	scn2681_channelB_rx_rsr_ticks = 0;
	scn2681_channelB_rx_rsr_empty = true;
	scn2681_channelB_rx_status_overrun = false;
	io_duart_core_channel_fd_set_RTS(scn2681_channelB_serial_device_fd);
	scn2681_channelB_rx_rts_enabled = true;
}

// Tx
void io_duart_core_channelB_tx_enable() {
	scn2681_channelB_tx_enabled = true;
}
void io_duart_core_channelB_tx_disable() {
	scn2681_channelB_tx_enabled = false;
}
void io_duart_core_channelB_tx_reset() {
	io_duart_core_channelB_tx_disable();
	scn2681_channelB_tx_thr = 0;
	scn2681_channelB_tx_thr_empty = true;
	scn2681_channelB_tx_tsr = 0;
	scn2681_channelB_tx_tsr_empty = true;
	scn2681_channelB_tx_tsr_ticks = 0;
}

// Initialise (reset) DUART core
//////////////////////////////////////////////////////////////////////////////////////////////
void io_duart_core_init() {
	// Channel A Registers
	scn2681_channelA_mode1 = 0;
	scn2681_channelA_mode2 = 0;
	scn2681_channelA_status = 0;
	scn2681_channelA_clock_select = 0;
	scn2681_channelA_command = 0;
	io_duart_core_channelA_reset_selected_mr();
	io_duart_core_channelA_rx_disable();
	io_duart_core_channelA_rx_reset();
	io_duart_core_channelA_tx_disable();
	io_duart_core_channelA_tx_reset();
	// Channel B Registers
	scn2681_channelB_mode1 = 0;
	scn2681_channelB_mode2 = 0;
	scn2681_channelB_status = 0;
	scn2681_channelB_clock_select = 0;
	scn2681_channelB_command = 0;
	io_duart_core_channelB_reset_selected_mr();
	io_duart_core_channelB_rx_disable();
	io_duart_core_channelB_rx_reset();
	io_duart_core_channelB_tx_disable();
	io_duart_core_channelB_tx_reset();
	// Input Port
	scn2681_input_port = 0;
	scn2681_input_port_change = 0;
	// Output Port
	scn2681_output_port_configuration = 0;
	scn2681_output_port_set = 0;
	scn2681_output_port_reset = 0;
	// Interrupts
	scn2681_interrupt_mask = 0;
	scn2681_interrupt_status = 0;
	// Misc
	scn2681_auxiliary_control = 0;
	// Counter / Timer
	scn2681_counter_timer_upper = 0;
	scn2681_counter_timer_lower = 0;
	scn2681_counter_timer_upper_preset = 0;
	scn2681_counter_timer_lower_preset = 0;
	scn2681_counter_start_command = 0;
	scn2681_counter_stop_command = 0;
}

// Emulate a clock cycle of the DUART core
//////////////////////////////////////////////////////////////////////////////////////////////
void io_duart_core_clock_tick() {
	char	tmp_buffer;

	// Channel A Registers
	// Read one byte (if available) from serial device
	if (scn2681_channelA_rx_rsr_ticks > C2503_IO_DUART_CORE_TICKS_PER_BYTE) {
		if (scn2681_channelA_rx_rts_enabled &&
				(scn2681_channelA_serial_device_fd != -1) && (read(scn2681_channelA_serial_device_fd, &tmp_buffer, 1) > 0)) {
			// Check if there's already a byte in the receive shift register
			if (!scn2681_channelA_rx_rsr_empty) {
				scn2681_channelA_rx_status_overrun = true;
			}
			scn2681_channelA_rx_rsr = tmp_buffer;
			scn2681_channelA_rx_rsr_empty = false;

			scn2681_channelA_rx_rsr_ticks = 0;
		}
	} else {
		scn2681_channelA_rx_rsr_ticks++;
	}
	// Check whether to set RTS signal
	if (scn2681_channelA_mode1 & SCN2681_REG_MODE1_RX_RTS) {
		if (!scn2681_channelA_rx_rsr_empty) {
			// Check if the FIFO is full of valid bytes
			if (scn2681_channelA_rx_fifo[scn2681_channelA_rx_wr_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE) {
				if (scn2681_channelA_rx_rts_enabled) {
					io_duart_core_channel_fd_clear_RTS(scn2681_channelA_serial_device_fd);
					scn2681_channelA_rx_rts_enabled = false;
				}
			} else {
				if (!scn2681_channelA_rx_rts_enabled) {
					io_duart_core_channel_fd_set_RTS(scn2681_channelA_serial_device_fd);
					scn2681_channelA_rx_rts_enabled = true;
				}
			}
		}
	}
	// Push Rx shift register to FIFO
	if (!scn2681_channelA_rx_rsr_empty) {
		// Check if there is already a byte in the next buffer
		if (!(scn2681_channelA_rx_fifo[scn2681_channelA_rx_wr_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE)) {
			scn2681_channelA_rx_fifo[scn2681_channelA_rx_wr_idx] = scn2681_channelA_rx_rsr | SCN2681_REG_RX_FIFO_VALID_BYTE;
			// Increment FIFO index
			scn2681_channelA_rx_wr_idx++;
			// Reset FIFO index if it overflows
			if (scn2681_channelA_rx_wr_idx >= SCN2681_REG_RX_FIFO_SIZE) scn2681_channelA_rx_wr_idx = 0;
			scn2681_channelA_rx_rsr_empty = true;
		}
	}
	// Push Tx shift register to serial device
	if (!scn2681_channelA_tx_tsr_empty) {
		scn2681_channelA_tx_tsr_ticks++;
		if (scn2681_channelA_tx_tsr_ticks > C2503_IO_DUART_CORE_TICKS_PER_BYTE) {
			// Finished sending byte serially
			if (scn2681_channelA_serial_device_fd != -1) {
				write(scn2681_channelA_serial_device_fd, &scn2681_channelA_tx_tsr, 1);
			}
			scn2681_channelA_tx_tsr_empty = true;
			scn2681_channelA_tx_tsr_ticks = 0;
		}
	}
	// Move Tx holding register to shift register
	if (!scn2681_channelA_tx_thr_empty) {
		if (scn2681_channelA_tx_tsr_empty) {
			scn2681_channelA_tx_tsr = scn2681_channelA_tx_thr;
			scn2681_channelA_tx_tsr_empty = false;
			scn2681_channelA_tx_thr_empty = true;
		}
	}

	// Channel B Registers
	// Read one byte (if available) from serial device
	if (scn2681_channelB_rx_rsr_ticks > C2503_IO_DUART_CORE_TICKS_PER_BYTE) {
		if (scn2681_channelB_rx_rts_enabled &&
				(scn2681_channelB_serial_device_fd != -1) && (read(scn2681_channelB_serial_device_fd, &tmp_buffer, 1) > 0)) {
			// Check if there's already a byte in the receive shift register
			if (!scn2681_channelB_rx_rsr_empty) {
				scn2681_channelB_rx_status_overrun = true;
			}
			scn2681_channelB_rx_rsr = tmp_buffer;
			scn2681_channelB_rx_rsr_empty = false;

			scn2681_channelB_rx_rsr_ticks = 0;
		}
	} else {
		scn2681_channelB_rx_rsr_ticks++;
	}
	// Check whether to set RTS signal
	if (scn2681_channelB_mode1 & SCN2681_REG_MODE1_RX_RTS) {
		if (!scn2681_channelB_rx_rsr_empty) {
			// Check if the FIFO is full of valid bytes
			if (scn2681_channelB_rx_fifo[scn2681_channelB_rx_wr_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE) {
				if (scn2681_channelB_rx_rts_enabled) {
					io_duart_core_channel_fd_clear_RTS(scn2681_channelB_serial_device_fd);
					scn2681_channelB_rx_rts_enabled = false;
				}
			} else {
				if (!scn2681_channelB_rx_rts_enabled) {
					io_duart_core_channel_fd_set_RTS(scn2681_channelB_serial_device_fd);
					scn2681_channelB_rx_rts_enabled = true;
				}
			}
		}
	}
	// Push Rx shift register to FIFO
	if (!scn2681_channelB_rx_rsr_empty) {
		scn2681_channelB_rx_rsr_ticks++;
		if (scn2681_channelB_rx_rsr_ticks > C2503_IO_DUART_CORE_TICKS_PER_BYTE) {
			// Check if there is already a byte in the next buffer
			if (!(scn2681_channelB_rx_fifo[scn2681_channelB_rx_wr_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE)) {
				scn2681_channelB_rx_fifo[scn2681_channelB_rx_wr_idx] = scn2681_channelB_rx_rsr | SCN2681_REG_RX_FIFO_VALID_BYTE;
				// Increment FIFO index
				scn2681_channelB_rx_wr_idx++;
				// Reset FIFO index if it overflows
				if (scn2681_channelB_rx_wr_idx >= SCN2681_REG_RX_FIFO_SIZE) scn2681_channelB_rx_wr_idx = 0;
				scn2681_channelB_rx_rsr_empty = true;
				scn2681_channelB_rx_rsr_ticks = 0;
			}
		}
	}
	// Push Tx shift register to serial device
	if (!scn2681_channelB_tx_tsr_empty) {
		scn2681_channelB_tx_tsr_ticks++;
		if (scn2681_channelB_tx_tsr_ticks > C2503_IO_DUART_CORE_TICKS_PER_BYTE) {
			// Finished sending byte serially
			if (scn2681_channelB_serial_device_fd != -1) {
				write(scn2681_channelB_serial_device_fd, &scn2681_channelB_tx_tsr, 1);
			}
			scn2681_channelB_tx_tsr_empty = true;
			scn2681_channelB_tx_tsr_ticks = 0;
		}
	}
	// Move Tx holding register to shift register
	if (!scn2681_channelB_tx_thr_empty) {
		if (scn2681_channelB_tx_tsr_empty) {
			scn2681_channelB_tx_tsr = scn2681_channelB_tx_thr;
			scn2681_channelB_tx_tsr_empty = false;
			scn2681_channelB_tx_thr_empty = true;
		}
	}
}

// Read a byte from address in to value from the DUART core
// Some register update when read, when real_read is false disables that behaviour (needed for memory dump display)
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_duart_read_byte(unsigned int address, unsigned int *value, bool real_read) {
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		switch (address - C2503_IO_DUART_ADDR) {
			case SCN2681_REG_RD_MODE_A:				// Channel A: Mode Register 1/2
				if (scn2681_channelA_mode2_selected) {
					*value = scn2681_channelA_mode2;
				} else {
					*value = scn2681_channelA_mode1;
				}
				if (real_read & !scn2681_channelA_mode2_selected) scn2681_channelA_mode2_selected = true;
				break;
			case SCN2681_REG_RD_STATUS_A:			// Channel A: Status Register
				*value = scn2681_channelA_status;
				if (scn2681_channelA_rx_fifo[scn2681_channelA_rx_rd_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE) *value = *value | SCN2681_REG_STATUS_RX_RDY;
				if ((scn2681_channelA_rx_rd_idx == scn2681_channelA_rx_wr_idx) && (scn2681_channelA_rx_fifo[scn2681_channelA_rx_rd_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE)) *value = *value | SCN2681_REG_STATUS_FFULL;
				if (scn2681_channelA_tx_thr_empty) *value = *value | SCN2681_REG_STATUS_TX_RDY;
				if (scn2681_channelA_tx_thr_empty & scn2681_channelA_tx_tsr_empty) *value = *value | SCN2681_REG_STATUS_TX_EMT;
				if (scn2681_channelA_rx_status_overrun) *value = *value | SCN2681_REG_STATUS_OVERRUN_ERROR;
				break;
			case SCN2681_REG_RD_BRG_TEST:			// Baud Rate Generator Test
				*value = 0;				// Not Implemented
				break;
			case SCN2681_REG_RD_RX_A:			// Channel A: RX Register
				*value = 0;
				if (scn2681_channelA_rx_fifo[scn2681_channelA_rx_rd_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE) {
					// Get value, stripping status flags
					*value = 0xff & scn2681_channelA_rx_fifo[scn2681_channelA_rx_rd_idx];
					if (real_read) {
						// Reset flags
						scn2681_channelA_rx_fifo[scn2681_channelA_rx_rd_idx] = *value;
						// Increment index
						scn2681_channelA_rx_rd_idx++;
						// Reset FIFO index if it overflows
						if (scn2681_channelA_rx_rd_idx > 2) scn2681_channelA_rx_rd_idx = 0;
					}
				}
				break;
			case SCN2681_REG_RD_INPUT_PORT_CHANGE:		// Input Port Change Register
				*value = scn2681_input_port_change;
				break;
			case SCN2681_REG_RD_INTERRUPT_STATUS:		// Interrupt Status Register
				*value = scn2681_interrupt_status;
				break;
			case SCN2681_REG_RD_CTR_UPPER_VALUE:		// Counter/Timer Upper Value Register
				*value = scn2681_counter_timer_upper;
				break;
			case SCN2681_REG_RD_CTR_LOWER_VALUE:		// Counter/Timer Lower Value Register
				*value = scn2681_counter_timer_lower;
				break;
			case SCN2681_REG_RD_MODE_B:			// Channel B: Mode Register 1/2
				if (scn2681_channelB_mode2_selected) {
					*value = scn2681_channelB_mode2;
				} else {
					*value = scn2681_channelB_mode1;
				}
				if (real_read & !scn2681_channelB_mode2_selected) scn2681_channelB_mode2_selected = true;
				break;
			case SCN2681_REG_RD_STATUS_B:			// Channel B: Status Register
				*value = scn2681_channelB_status;
				if (scn2681_channelB_rx_fifo[scn2681_channelB_rx_rd_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE) *value = *value | SCN2681_REG_STATUS_RX_RDY;
				if ((scn2681_channelB_rx_rd_idx == scn2681_channelB_rx_wr_idx) && (scn2681_channelB_rx_fifo[scn2681_channelB_rx_rd_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE)) *value = *value | SCN2681_REG_STATUS_FFULL;
				if (scn2681_channelB_tx_thr_empty) *value = *value | SCN2681_REG_STATUS_TX_RDY;
				if (scn2681_channelB_tx_thr_empty & scn2681_channelB_tx_tsr_empty) *value = *value | SCN2681_REG_STATUS_TX_EMT;
				if (scn2681_channelB_rx_status_overrun) *value = *value | SCN2681_REG_STATUS_OVERRUN_ERROR;
				break;
			case SCN2681_REG_RD_1X16X_TEST:			// 1X/16X Test
				*value = 0;				// Not Implemented
				break;
			case SCN2681_REG_RD_RX_B:			// Channel B: RX Register
				*value = 0;
				if (scn2681_channelB_rx_fifo[scn2681_channelB_rx_rd_idx] & SCN2681_REG_RX_FIFO_VALID_BYTE) {
					// Get value, stripping status flags
					*value = 0xff & scn2681_channelB_rx_fifo[scn2681_channelB_rx_rd_idx];
					if (real_read) {
						// Reset flags
						scn2681_channelB_rx_fifo[scn2681_channelB_rx_rd_idx] = *value;
						// Increment index
						scn2681_channelB_rx_rd_idx++;
						// Reset FIFO index if it overflows
						if (scn2681_channelB_rx_rd_idx > 2) scn2681_channelB_rx_rd_idx = 0;
					}
				}
				break;
			case SCN2681_REG_RD_RESERVED:			// Reserved
				*value = 0;				// Not Implemented
				break;
			case SCN2681_REG_RD_INPUT_PORT:			// Input Port Register
				*value = scn2681_input_port;
				break;
			case SCN2681_REG_RD_CTR_START_CMD:		// Counter Start Command Register
				*value = scn2681_counter_start_command;
				break;
			case SCN2681_REG_RD_CTR_STOP_CMD:			// Counter Stop Command Register
				*value = scn2681_counter_stop_command;
				break;
		}

		return true;
	}
	return false;
}

// Write value to address in the DUART core
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_duart_write_byte(unsigned int address, unsigned int value) {
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		switch (address - C2503_IO_DUART_ADDR) {
			case SCN2681_REG_WR_MODE_A:			// Channel A: Mode Register 1/2
				if (scn2681_channelA_mode2_selected) {
					scn2681_channelA_mode2 = value;
				} else {
					// Only bother updating serial settings if they've changed
					if ((value ^ scn2681_channelA_mode1) & 0x1F) {
						io_duart_core_channel_fd_set_mode1(scn2681_channelA_serial_device_fd, value);
					}
					scn2681_channelA_mode1 = value;
				}
				if (!scn2681_channelA_mode2_selected) scn2681_channelA_mode2_selected = true;
				break;
			case SCN2681_REG_WR_CLOCK_SELECT_A:		// Channel A: Clock Select Register
				scn2681_channelA_clock_select = value;
				io_duart_core_channel_fd_set_baud(scn2681_channelA_serial_device_fd,
									scn2681_channelA_clock_select,
									scn2681_auxiliary_control & SCN2681_REG_AUX_CONTROL_BRG_SET_SELECT);
				break;
			case SCN2681_REG_WR_COMMAND_A:			// Channel A: Command Register
				scn2681_channelA_command = value;
				if (scn2681_channelA_command & SCN2681_REG_COMMAND_RX_ENABLE) io_duart_core_channelA_rx_enable();
				if (scn2681_channelA_command & SCN2681_REG_COMMAND_RX_DISABLE) io_duart_core_channelA_rx_disable();
				if (scn2681_channelA_command & SCN2681_REG_COMMAND_TX_ENABLE) io_duart_core_channelA_tx_enable();
				if (scn2681_channelA_command & SCN2681_REG_COMMAND_TX_DISABLE) io_duart_core_channelA_tx_disable();
				if ((scn2681_channelA_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_MR) io_duart_core_channelA_reset_selected_mr();
				if ((scn2681_channelA_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_RX) io_duart_core_channelA_rx_reset();
				if ((scn2681_channelA_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_TX) io_duart_core_channelA_tx_reset();
				if ((scn2681_channelA_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_ERR) io_duart_core_channelA_reset_error_status();
				// Not Implemented
				// SCN2681_REG_COMMAND_MISC_RESET_CHNG
				// SCN2681_REG_COMMAND_MISC_BREAK_START
				// SCN2681_REG_COMMAND_MISC_BREAK_STOP
				break;
			case SCN2681_REG_WR_TX_A:			// Channel A: TX Register
				if (scn2681_channelA_tx_enabled) {
					scn2681_channelA_tx_thr = value;
					scn2681_channelA_tx_thr_empty = false;
				}
				break;
			case SCN2681_REG_WR_AUX_CONTROL:		// Auxiliary Control Register
				scn2681_auxiliary_control = value;
				break;
			case SCN2681_REG_WR_INTERRUPT_MASK:		// Interrupt Mask Register
				scn2681_interrupt_mask = value;
				break;
			case SCN2681_REG_WR_CTR_UPPER_VALUE_PRESET:	// Counter/Timer Upper Value Preset Register
				scn2681_counter_timer_upper_preset = value;
				break;
			case SCN2681_REG_WR_CTR_LOWER_VALUE_PRESET:	// Counter/Timer Lower Value Preset Register
				scn2681_counter_timer_lower_preset = value;
				break;
			case SCN2681_REG_WR_MODE_B:			// Channel B: Mode Register 1/2
				if (scn2681_channelB_mode2_selected) {
					scn2681_channelB_mode2 = value;
				} else {
					// Only bother updating serial settings if they've changed
					if ((value ^ scn2681_channelB_mode1) & 0x1F) {
						io_duart_core_channel_fd_set_mode1(scn2681_channelB_serial_device_fd, value);
					}
					scn2681_channelB_mode1 = value;
				}
				if (!scn2681_channelB_mode2_selected) scn2681_channelB_mode2_selected = true;
				break;
			case SCN2681_REG_WR_CLOCK_SELECT_B:		// Channel B: Clock Select Register
				scn2681_channelB_clock_select = value;
				io_duart_core_channel_fd_set_baud(scn2681_channelB_serial_device_fd,
									scn2681_channelB_clock_select,
									scn2681_auxiliary_control & SCN2681_REG_AUX_CONTROL_BRG_SET_SELECT);
				break;
			case SCN2681_REG_WR_COMMAND_B:			// Channel B: Command Register
				scn2681_channelB_command = value;
				if (scn2681_channelB_command & SCN2681_REG_COMMAND_RX_ENABLE) io_duart_core_channelB_rx_enable();
				if (scn2681_channelB_command & SCN2681_REG_COMMAND_RX_DISABLE) io_duart_core_channelB_rx_disable();
				if (scn2681_channelB_command & SCN2681_REG_COMMAND_TX_ENABLE) io_duart_core_channelB_tx_enable();
				if (scn2681_channelB_command & SCN2681_REG_COMMAND_TX_DISABLE) io_duart_core_channelB_tx_disable();
				if ((scn2681_channelB_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_MR) io_duart_core_channelB_reset_selected_mr();
				if ((scn2681_channelB_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_RX) io_duart_core_channelB_rx_reset();
				if ((scn2681_channelB_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_TX) io_duart_core_channelB_tx_reset();
				if ((scn2681_channelB_command & 0xF0) == SCN2681_REG_COMMAND_MISC_RESET_ERR) io_duart_core_channelB_reset_error_status();
				// Not Implemented
				// SCN2681_REG_COMMAND_MISC_RESET_CHNG
				// SCN2681_REG_COMMAND_MISC_BREAK_START
				// SCN2681_REG_COMMAND_MISC_BREAK_STOP
				break;
			case SCN2681_REG_WR_TX_B:			// Channel B: TX Register
				if (scn2681_channelB_tx_enabled) {
					scn2681_channelB_tx_thr = value;
					scn2681_channelB_tx_thr_empty = false;
				}
				break;
			case SCN2681_REG_WR_RESERVED:			// Reserved
									// Not Implemented
				break;
			case SCN2681_REG_WR_OUTPUT_PORT_CONF:		// Output Port Configuration Register
				scn2681_output_port_configuration = value;
				break;
			case SCN2681_REG_WR_OUTPUT_PORT_SET:		// Output Port Set Register
				scn2681_output_port_set = value;
				break;
			case SCN2681_REG_WR_OUTPUT_PORT_RESET:		// Output Port Reset Register
				scn2681_output_port_reset = value;
				break;
		}

		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Channel A: LANCE AM79C90
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned short	g_io_chnla_lance_rap;
unsigned short	g_io_chnla_lance_csr1;
unsigned short	g_io_chnla_lance_csr2;
unsigned short	g_io_chnla_lance_csr3;

unsigned int	g_io_chnla_lance_iblock_start;
unsigned char	g_io_chnla_lance_iblock_offset;

bool		g_io_chnla_lance_csr0_babl, \
		g_io_chnla_lance_csr0_cerr, \
		g_io_chnla_lance_csr0_miss, \
		g_io_chnla_lance_csr0_merr, \
		g_io_chnla_lance_csr0_rint, \
		g_io_chnla_lance_csr0_tint, \
		g_io_chnla_lance_csr0_idon, \
		g_io_chnla_lance_csr0_inea, \
		g_io_chnla_lance_csr0_rxon, \
		g_io_chnla_lance_csr0_txon, \
		g_io_chnla_lance_csr0_tdmd, \
		g_io_chnla_lance_csr0_stop, \
		g_io_chnla_lance_csr0_strt, \
		g_io_chnla_lance_csr0_init;

/* Equivalent to device reset or STOP */
void io_channela_reset() {
	g_io_chnla_lance_rap = 0;
	g_io_chnla_lance_csr3 = 0;

	g_io_chnla_lance_csr0_babl = false;
	g_io_chnla_lance_csr0_cerr = false;
	g_io_chnla_lance_csr0_miss = false;
	g_io_chnla_lance_csr0_merr = false;
	g_io_chnla_lance_csr0_rint = false;
	g_io_chnla_lance_csr0_tint = false;
	g_io_chnla_lance_csr0_idon = false;
	g_io_chnla_lance_csr0_inea = false;
	g_io_chnla_lance_csr0_rxon = false;
	g_io_chnla_lance_csr0_txon = false;
	g_io_chnla_lance_csr0_tdmd = false;
	g_io_chnla_lance_csr0_strt = false;
	g_io_chnla_lance_csr0_init = false;

	g_io_chnla_lance_csr0_stop = true;
}

void io_channela_core_init() {
	g_io_chnla_lance_csr1 = 0;
	g_io_chnla_lance_csr2 = 0;
	g_io_chnla_lance_iblock_start = 0x00000000;

	io_channela_reset();
}

/* Events */
void io_channela_event_babble() {
	if (!g_io_chnla_lance_csr0_babl) {				// Check if not already set
		g_io_chnla_lance_csr0_babl = true;			// Set flag
		if (g_io_chnla_lance_csr0_inea) {			// Check whether interrupts are enabled
//			m68k_set_irq();					// Trigger interrupt
		}
	}
}

void io_channela_event_missed_packet() {
	if (!g_io_chnla_lance_csr0_miss) {				// Check if not already set
		g_io_chnla_lance_csr0_miss = true;			// Set flag
		if (g_io_chnla_lance_csr0_inea) {			// Check whether interrupts are enabled
//			m68k_set_irq();					// Trigger interrupt
		}
	}
}

void io_channela_event_memory_error() {
	if (!g_io_chnla_lance_csr0_merr) {				// Check if not already set
		g_io_chnla_lance_csr0_merr = true;			// Set flag
		if (g_io_chnla_lance_csr0_inea) {			// Check whether interrupts are enabled
//			m68k_set_irq();					// Trigger interrupt
		}
	}
}

void io_channela_event_rx_interrupt() {
	if (!g_io_chnla_lance_csr0_rint) {				// Check if not already set
		g_io_chnla_lance_csr0_rint = true;			// Set flag
		if (g_io_chnla_lance_csr0_inea) {			// Check whether interrupts are enabled
//			m68k_set_irq();					// Trigger interrupt
		}
	}
}

void io_channela_event_tx_interrupt() {
	if (!g_io_chnla_lance_csr0_tint) {				// Check if not already set
		g_io_chnla_lance_csr0_tint = true;			// Set flag
		if (g_io_chnla_lance_csr0_inea) {			// Check whether interrupts are enabled
//			m68k_set_irq();					// Trigger interrupt
		}
	}
}

void io_channela_event_init_done() {
	if (!g_io_chnla_lance_csr0_idon) {				// Check if not already set
		g_io_chnla_lance_csr0_idon = true;			// Set flag
		if (g_io_chnla_lance_csr0_inea) {			// Check whether interrupts are enabled
//			m68k_set_irq();					// Trigger interrupt
		}
	}
}

void io_channela_core_clock_tick() {
	// Configure LANCE if INIT enabled
	if (g_io_chnla_lance_csr0_init) {
		/* Process initialisation block */
//		switch (g_io_chnla_lance_iblock_offset) {
//			....

		if (g_io_chnla_lance_iblock_offset == 0x18) {
			io_channela_event_init_done();			// Set CSR0 flag and trigger interrupt
		} else {
			/* Increment init block pointer */
			g_io_chnla_lance_iblock_offset += 0x02;
		}
	}
}

bool io_channela_isIRQ() {
	return g_io_chnla_lance_csr0_babl | g_io_chnla_lance_csr0_miss | g_io_chnla_lance_csr0_merr | \
				g_io_chnla_lance_csr0_rint | g_io_chnla_lance_csr0_tint | g_io_chnla_lance_csr0_idon;
}

bool io_channela_read_word(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_1_reset_channela) return false;	// If device is currently reset, access is not allowed

	if (address == C2503_IO_CHANNELA_LANCE_RDP_ADDR) {
		switch (g_io_chnla_lance_rap) {
			case 0:
				if (g_io_chnla_lance_csr0_babl | g_io_chnla_lance_csr0_cerr | \
					g_io_chnla_lance_csr0_miss | g_io_chnla_lance_csr0_merr) *value |= 0x8000;
				if (g_io_chnla_lance_csr0_babl) *value |= 0x4000;
				if (g_io_chnla_lance_csr0_cerr) *value |= 0x2000;
				if (g_io_chnla_lance_csr0_miss) *value |= 0x1000;
				if (g_io_chnla_lance_csr0_merr) *value |= 0x0800;
				if (g_io_chnla_lance_csr0_rint) *value |= 0x0400;
				if (g_io_chnla_lance_csr0_tint) *value |= 0x0200;
				if (g_io_chnla_lance_csr0_idon) *value |= 0x0100;
				if (io_channela_isIRQ()) *value |= 0x0080;
				if (g_io_chnla_lance_csr0_inea) *value |= 0x0040;
				if (g_io_chnla_lance_csr0_rxon) *value |= 0x0020;
				if (g_io_chnla_lance_csr0_txon) *value |= 0x0010;
				if (g_io_chnla_lance_csr0_tdmd) *value |= 0x0008;
				if (g_io_chnla_lance_csr0_stop) *value |= 0x0004;
				if (g_io_chnla_lance_csr0_strt) *value |= 0x0002;
				if (g_io_chnla_lance_csr0_init) *value |= 0x0001;

				break;
			case 1:
				*value = g_io_chnla_lance_csr1;
				break;
			case 2:
				*value = g_io_chnla_lance_csr2;
				break;
			case 3:
				*value = g_io_chnla_lance_csr3;
				break;
		}
		return true;
	}
	if (address == C2503_IO_CHANNELA_LANCE_RAP_ADDR) {
		*value = g_io_chnla_lance_rap;
		return true;
	}
	return false;
}

bool io_channela_write_word(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_1_reset_channela) return false;	// If device is currently reset, access is not allowed

	if (address == C2503_IO_CHANNELA_LANCE_RDP_ADDR) {
		switch (g_io_chnla_lance_rap) {
			case 0:
				if (value & 0x4000) g_io_chnla_lance_csr0_babl = false;
				if (value & 0x2000) g_io_chnla_lance_csr0_cerr = false;
				if (value & 0x1000) g_io_chnla_lance_csr0_miss = false;
				if (value & 0x0800) g_io_chnla_lance_csr0_merr = false;
				if (value & 0x0400) g_io_chnla_lance_csr0_rint = false;
				if (value & 0x0200) g_io_chnla_lance_csr0_tint = false;
				if (value & 0x0100) g_io_chnla_lance_csr0_idon = false;
				g_io_chnla_lance_csr0_inea = (value & 0x0040) ? true : false;
				if (value & 0x0008) g_io_chnla_lance_csr0_tdmd = true;
				if ((value & 0x0002) & g_io_chnla_lance_csr0_stop) g_io_chnla_lance_csr0_strt = true;
				if ((value & 0x0001) & g_io_chnla_lance_csr0_stop) {
					// Configure init block address
					g_io_chnla_lance_iblock_start = ((g_io_chnla_lance_csr2 && 0x00ff) << 16) | g_io_chnla_lance_csr1;
					// Reset init block pointer offset
					g_io_chnla_lance_iblock_offset = 0x00;
					// Reset STOP flag
					g_io_chnla_lance_csr0_stop = false;
					// Set INIT flag
					g_io_chnla_lance_csr0_init = true;
				}

				/* STOP overrides other bits */
				if (value & 0x0004) io_channela_reset();

				break;
			case 1:
				g_io_chnla_lance_csr1 = value & 0xfffe;
				break;
			case 2:
				g_io_chnla_lance_csr2 = value & 0x00ff;
				break;
			case 3:
				g_io_chnla_lance_csr3 = value & 0x7;
				break;
		}
		return true;
	}
	if (address == C2503_IO_CHANNELA_LANCE_RAP_ADDR) {
		g_io_chnla_lance_rap = value;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Channel B: Serial (HD64570)
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_chnlb_serial[C2503_IO_CHANNELB_SERIAL_SIZE];

bool io_channelb_isIRQ() {
	return false;
}

bool io_channelb_read_byte(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_2_reset_channelb) return false;	// If device is currently reset, access is not allowed

	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		*value = READ_BYTE(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR);
		return true;
	}
	return false;
}

bool io_channelb_read_word(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_2_reset_channelb) return false;	// If device is currently reset, access is not allowed

	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		*value = READ_WORD(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR);
		return true;
	}
	return false;
}

bool io_channelb_read_long(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_2_reset_channelb) return false;	// If device is currently reset, access is not allowed

	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		*value = READ_LONG(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR);
		return true;
	}
	return false;
}

bool io_channelb_write_byte(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_2_reset_channelb) return false;	// If device is currently reset, access is not allowed

	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		WRITE_BYTE(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR, value);
		return true;
	}
	return false;
}

bool io_channelb_write_word(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_2_reset_channelb) return false;	// If device is currently reset, access is not allowed

	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		WRITE_WORD(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR, value);
		return true;
	}
	return false;
}

bool io_channelb_write_long(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read
	if (g_io_sysctrl05_2_reset_channelb) return false;	// If device is currently reset, access is not allowed

	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		WRITE_LONG(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR, value);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Unknown1 device
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_unknown1[C2503_IO_UNKNOWN1_SIZE];

bool io_unknown1_read_byte(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

	if ((address >= C2503_IO_UNKNOWN1_ADDR) && (address < (C2503_IO_UNKNOWN1_ADDR + C2503_IO_UNKNOWN1_SIZE))) {
		*value = READ_BYTE(g_io_unknown1, address - C2503_IO_UNKNOWN1_ADDR);
		return true;
	}
	return false;
}

bool io_unknown1_read_word(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

#if C2503_IO_UNKNOWN1_SIZE >= 2
	if ((address >= C2503_IO_UNKNOWN1_ADDR) && (address < (C2503_IO_UNKNOWN1_ADDR + C2503_IO_UNKNOWN1_SIZE))) {
		*value = READ_WORD(g_io_unknown1, address - C2503_IO_UNKNOWN1_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_unknown1_read_long(unsigned int address, unsigned int *value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

#if C2503_IO_UNKNOWN1_SIZE >= 4
	if ((address >= C2503_IO_UNKNOWN1_ADDR) && (address < (C2503_IO_UNKNOWN1_ADDR + C2503_IO_UNKNOWN1_SIZE))) {
		*value = READ_LONG(g_io_unknown1, address - C2503_IO_UNKNOWN1_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_unknown1_write_byte(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

	if ((address >= C2503_IO_UNKNOWN1_ADDR) && (address < (C2503_IO_UNKNOWN1_ADDR + C2503_IO_UNKNOWN1_SIZE))) {
		WRITE_BYTE(g_io_unknown1, address - C2503_IO_UNKNOWN1_ADDR, value);
		return true;
	}
	return false;
}

bool io_unknown1_write_word(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

#if C2503_IO_UNKNOWN1_SIZE >= 2
	if ((address >= C2503_IO_UNKNOWN1_ADDR) && (address < (C2503_IO_UNKNOWN1_ADDR + C2503_IO_UNKNOWN1_SIZE))) {
		WRITE_WORD(g_io_unknown1, address - C2503_IO_UNKNOWN1_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_unknown1_write_long(unsigned int address, unsigned int value) {
	if (sri_enabled) return false;				// If SRI enabled, bypass read

#if C2503_IO_UNKNOWN1_SIZE >= 4
	if ((address >= C2503_IO_UNKNOWN1_ADDR) && (address < (C2503_IO_UNKNOWN1_ADDR + C2503_IO_UNKNOWN1_SIZE))) {
		WRITE_LONG(g_io_unknown1, address - C2503_IO_UNKNOWN1_ADDR, value);
		return true;
	}
#endif
	return false;
}
