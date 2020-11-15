#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
//#include <time.h>
#include <unistd.h>
#include "cisco_2503.h"
#include "cisco_2503_peripherals.h"

// Memory
//////////////////////////////////////////////////////////////////////////////////////////////
// Boot ROM
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_bootrom[C2503_BOOTROM_SIZE];

// Initialise boot flash ROM with contents from file
bool mem_bootrom_init(FILE *fhandle) {
	if (fread(g_bootrom, 1, C2503_BOOTROM_SIZE, fhandle) <= 0) return false;
	return true;
}

bool mem_bootrom_read_byte(unsigned address, unsigned int *value) {
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE))) {
		*value = READ_BYTE(g_bootrom, address - C2503_BOOTROM_ADDR1);
		return true;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		*value = READ_BYTE(g_bootrom, address - C2503_BOOTROM_ADDR2);
		return true;
	}
	return false;
}

bool mem_bootrom_read_word(unsigned address, unsigned int *value) {
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE))) {
		*value = READ_WORD(g_bootrom, address - C2503_BOOTROM_ADDR1);
		return true;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		*value = READ_WORD(g_bootrom, address - C2503_BOOTROM_ADDR2);
		return true;
	}
	return false;
}

bool mem_bootrom_read_long(unsigned address, unsigned int *value) {
	// Boot ROM Address 1
	if ((address >= C2503_BOOTROM_ADDR1) && (address < (C2503_BOOTROM_ADDR1 + C2503_BOOTROM_SIZE))) {
		*value = READ_LONG(g_bootrom, address - C2503_BOOTROM_ADDR1);
		return true;
	}
	// Boot ROM Address 2
	if ((address >= C2503_BOOTROM_ADDR2) && (address < (C2503_BOOTROM_ADDR2 + C2503_BOOTROM_SIZE))) {
		*value = READ_LONG(g_bootrom, address - C2503_BOOTROM_ADDR2);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// NVRAM
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_nvram[C2503_NVRAM_SIZE];

bool mem_nvram_read_byte(unsigned address, unsigned int *value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		*value = READ_BYTE(g_nvram, address - C2503_NVRAM_ADDR);
		return true;
	}
	return false;
}

bool mem_nvram_read_word(unsigned address, unsigned int *value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		*value = READ_WORD(g_nvram, address - C2503_NVRAM_ADDR);
		return true;
	}
	return false;
}

bool mem_nvram_read_long(unsigned address, unsigned int *value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		*value = READ_LONG(g_nvram, address - C2503_NVRAM_ADDR);
		return true;
	}
	return false;
}

bool mem_nvram_write_byte(unsigned address, unsigned int value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		WRITE_BYTE(g_nvram, address - C2503_NVRAM_ADDR, value);
		return true;
	}
	return false;
}

bool mem_nvram_write_word(unsigned address, unsigned int value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		WRITE_WORD(g_nvram, address - C2503_NVRAM_ADDR, value);
		return true;
	}
	return false;
}

bool mem_nvram_write_long(unsigned address, unsigned int value) {
	if ((address >= C2503_NVRAM_ADDR) && (address < (C2503_NVRAM_ADDR + C2503_NVRAM_SIZE))) {
		WRITE_LONG(g_nvram, address - C2503_NVRAM_ADDR, value);
		return true;
	}
	return false;
}

// I/O
//////////////////////////////////////////////////////////////////////////////////////////////
// 68302
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_68302_reg[C2503_IO_68302_REG_SIZE];
unsigned char g_io_68302_mem[C2503_IO_68302_RAM_SIZE];

bool io_68302_read_byte(unsigned address, unsigned int *value) {
	// 68302 registers
	if ((address >= C2503_IO_68302_REG_ADDR) && (address < (C2503_IO_68302_REG_ADDR + C2503_IO_68302_REG_SIZE))) {
		*value = READ_BYTE(g_io_68302_reg, address - C2503_IO_68302_REG_ADDR);
		return true;
	}
	// 68302 memory
	if ((address >= C2503_IO_68302_RAM_ADDR) && (address < (C2503_IO_68302_RAM_ADDR + C2503_IO_68302_RAM_SIZE))) {
		*value = READ_BYTE(g_io_68302_mem, address - C2503_IO_68302_RAM_ADDR);
		return true;
	}
	return false;
}

bool io_68302_read_word(unsigned address, unsigned int *value) {
#if C2503_IO_68302_REG_SIZE >= 2
	// 68302 registers
	if ((address >= C2503_IO_68302_REG_ADDR) && (address < (C2503_IO_68302_REG_ADDR + C2503_IO_68302_REG_SIZE))) {
		*value = READ_WORD(g_io_68302_reg, address - C2503_IO_68302_REG_ADDR);
		return true;
	}
#endif
#if C2503_IO_68302_RAM_SIZE >= 2
	// 68302 memory
	if ((address >= C2503_IO_68302_RAM_ADDR) && (address < (C2503_IO_68302_RAM_ADDR + C2503_IO_68302_RAM_SIZE))) {
		*value = READ_WORD(g_io_68302_mem, address - C2503_IO_68302_RAM_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_68302_read_long(unsigned address, unsigned int *value) {
#if C2503_IO_68302_REG_SIZE >= 4
	// 68302 registers
	if ((address >= C2503_IO_68302_REG_ADDR) && (address < (C2503_IO_68302_REG_ADDR + C2503_IO_68302_REG_SIZE))) {
		*value = READ_LONG(g_io_68302_reg, address - C2503_IO_68302_REG_ADDR);
		return true;
	}
#endif
#if C2503_IO_68302_RAM_SIZE >= 4
	// 68302 memory
	if ((address >= C2503_IO_68302_RAM_ADDR) && (address < (C2503_IO_68302_RAM_ADDR + C2503_IO_68302_RAM_SIZE))) {
		*value = READ_LONG(g_io_68302_mem, address - C2503_IO_68302_RAM_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_68302_write_byte(unsigned address, unsigned int value) {
	// 68302 registers
	if ((address >= C2503_IO_68302_REG_ADDR) && (address < (C2503_IO_68302_REG_ADDR + C2503_IO_68302_REG_SIZE))) {
		WRITE_BYTE(g_io_68302_reg, address - C2503_IO_68302_REG_ADDR, value);
		return true;
	}
	// 68302 memory
	if ((address >= C2503_IO_68302_RAM_ADDR) && (address < (C2503_IO_68302_RAM_ADDR + C2503_IO_68302_RAM_SIZE))) {
		WRITE_BYTE(g_io_68302_mem, address - C2503_IO_68302_RAM_ADDR, value);
		return true;
	}
	return false;
}

bool io_68302_write_word(unsigned address, unsigned int value) {
#if C2503_IO_68302_REG_SIZE >= 2
	// 68302 registers
	if ((address >= C2503_IO_68302_REG_ADDR) && (address < (C2503_IO_68302_REG_ADDR + C2503_IO_68302_REG_SIZE))) {
		WRITE_WORD(g_io_68302_reg, address - C2503_IO_68302_REG_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_68302_RAM_SIZE >= 2
	// 68302 memory
	if ((address >= C2503_IO_68302_RAM_ADDR) && (address < (C2503_IO_68302_RAM_ADDR + C2503_IO_68302_RAM_SIZE))) {
		WRITE_WORD(g_io_68302_mem, address - C2503_IO_68302_RAM_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_68302_write_long(unsigned address, unsigned int value) {
#if C2503_IO_68302_REG_SIZE >= 4
	// 68302 registers
	if ((address >= C2503_IO_68302_REG_ADDR) && (address < (C2503_IO_68302_REG_ADDR + C2503_IO_68302_REG_SIZE))) {
		WRITE_LONG(g_io_68302_reg, address - C2503_IO_68302_REG_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_68302_RAM_SIZE >= 4
	// 68302 memory
	if ((address >= C2503_IO_68302_RAM_ADDR) && (address < (C2503_IO_68302_RAM_ADDR + C2503_IO_68302_RAM_SIZE))) {
		WRITE_LONG(g_io_68302_mem, address - C2503_IO_68302_RAM_ADDR, value);
		return true;
	}
#endif
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// System Registers
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_sys_cntl1[C2503_IO_SYS_CONTROL1_SIZE];
unsigned char g_io_sys_cntl2[C2503_IO_SYS_CONTROL2_SIZE];
unsigned char g_io_sysid_cookie[C2503_IO_SYS_ID_COOKIE_SIZE];
unsigned char g_io_sys_status[C2503_IO_SYS_STATUS_SIZE];

bool io_system_read_byte(unsigned address, unsigned int *value) {
	// System control register 1
	if ((address >= C2503_IO_SYS_CONTROL1_ADDR) && (address < (C2503_IO_SYS_CONTROL1_ADDR + C2503_IO_SYS_CONTROL1_SIZE))) {
		*value = READ_BYTE(g_io_sys_cntl1, address - C2503_IO_SYS_CONTROL1_ADDR);
		return true;
	}
	// System control register 2
	if ((address >= C2503_IO_SYS_CONTROL2_ADDR) && (address < (C2503_IO_SYS_CONTROL2_ADDR + C2503_IO_SYS_CONTROL2_SIZE))) {
		*value = READ_BYTE(g_io_sys_cntl2, address - C2503_IO_SYS_CONTROL2_ADDR);
		return true;
	}
	// System ID cookie
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		*value = READ_BYTE(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR);
		return true;
	}
	// System status register
	if ((address >= C2503_IO_SYS_STATUS_ADDR) && (address < (C2503_IO_SYS_STATUS_ADDR + C2503_IO_SYS_STATUS_SIZE))) {
		*value = READ_BYTE(g_io_sys_status, address - C2503_IO_SYS_STATUS_ADDR);
		return true;
	}
	return false;
}

bool io_system_read_word(unsigned address, unsigned int *value) {
#if C2503_IO_SYS_CONTROL1_SIZE >= 2
	// System control register 1
	if ((address >= C2503_IO_SYS_CONTROL1_ADDR) && (address < (C2503_IO_SYS_CONTROL1_ADDR + C2503_IO_SYS_CONTROL1_SIZE))) {
		*value = READ_WORD(g_io_sys_cntl1, address - C2503_IO_SYS_CONTROL1_ADDR);
		return true;
	}
#endif
#if C2503_IO_SYS_CONTROL2_SIZE >= 2
	// System control register 2
	if ((address >= C2503_IO_SYS_CONTROL2_ADDR) && (address < (C2503_IO_SYS_CONTROL2_ADDR + C2503_IO_SYS_CONTROL2_SIZE))) {
		*value = READ_WORD(g_io_sys_cntl2, address - C2503_IO_SYS_CONTROL2_ADDR);
		return true;
	}
#endif
#if C2503_IO_SYS_ID_COOKIE_SIZE >= 2
	// System ID cookie
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		*value = READ_WORD(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR);
		return true;
	}
#endif
#if C2503_IO_SYS_STATUS_SIZE >= 2
	// System status register
	if ((address >= C2503_IO_SYS_STATUS_ADDR) && (address < (C2503_IO_SYS_STATUS_ADDR + C2503_IO_SYS_STATUS_SIZE))) {
		*value = READ_WORD(g_io_sys_status, address - C2503_IO_SYS_STATUS_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_system_read_long(unsigned address, unsigned int *value) {
#if C2503_IO_SYS_CONTROL1_SIZE >= 4
	// System control register 1
	if ((address >= C2503_IO_SYS_CONTROL1_ADDR) && (address < (C2503_IO_SYS_CONTROL1_ADDR + C2503_IO_SYS_CONTROL1_SIZE))) {
		*value = READ_LONG(g_io_sys_cntl1, address - C2503_IO_SYS_CONTROL1_ADDR);
		return true;
	}
#endif
#if C2503_IO_SYS_CONTROL2_SIZE >= 4
	// System control register 2
	if ((address >= C2503_IO_SYS_CONTROL2_ADDR) && (address < (C2503_IO_SYS_CONTROL2_ADDR + C2503_IO_SYS_CONTROL2_SIZE))) {
		*value = READ_LONG(g_io_sys_cntl2, address - C2503_IO_SYS_CONTROL2_ADDR);
		return true;
	}
#endif
#if C2503_IO_SYS_STATUS_SIZE >= 4
	// System status register
	if ((address >= C2503_IO_SYS_STATUS_ADDR) && (address < (C2503_IO_SYS_STATUS_ADDR + C2503_IO_SYS_STATUS_SIZE))) {
		*value = READ_LONG(g_io_sys_status, address - C2503_IO_SYS_STATUS_ADDR);
		return true;
	}
#endif
#if C2503_IO_SYS_ID_COOKIE_SIZE >= 4
	// System ID cookie
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		*value = READ_LONG(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_system_write_byte(unsigned address, unsigned int value) {
	// System control register 1
	if ((address >= C2503_IO_SYS_CONTROL1_ADDR) && (address < (C2503_IO_SYS_CONTROL1_ADDR + C2503_IO_SYS_CONTROL1_SIZE))) {
		WRITE_BYTE(g_io_sys_cntl1, address - C2503_IO_SYS_CONTROL1_ADDR, value);
		return true;
	}
	// System control register 2
	if ((address >= C2503_IO_SYS_CONTROL2_ADDR) && (address < (C2503_IO_SYS_CONTROL2_ADDR + C2503_IO_SYS_CONTROL2_SIZE))) {
		WRITE_BYTE(g_io_sys_cntl2, address - C2503_IO_SYS_CONTROL2_ADDR, value);
		return true;
	}
	// System status register
	if ((address >= C2503_IO_SYS_STATUS_ADDR) && (address < (C2503_IO_SYS_STATUS_ADDR + C2503_IO_SYS_STATUS_SIZE))) {
		WRITE_BYTE(g_io_sys_status, address - C2503_IO_SYS_STATUS_ADDR, value);
		return true;
	}
	// System ID cookie
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		WRITE_BYTE(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR, value);
		return true;
	}
	return false;
}

bool io_system_write_word(unsigned address, unsigned int value) {
#if C2503_IO_SYS_CONTROL1_SIZE >= 2
	// System control register 1
	if ((address >= C2503_IO_SYS_CONTROL1_ADDR) && (address < (C2503_IO_SYS_CONTROL1_ADDR + C2503_IO_SYS_CONTROL1_SIZE))) {
		WRITE_WORD(g_io_sys_cntl1, address - C2503_IO_SYS_CONTROL1_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_SYS_CONTROL2_SIZE >= 2
	// System control register 2
	if ((address >= C2503_IO_SYS_CONTROL2_ADDR) && (address < (C2503_IO_SYS_CONTROL2_ADDR + C2503_IO_SYS_CONTROL2_SIZE))) {
		WRITE_WORD(g_io_sys_cntl2, address - C2503_IO_SYS_CONTROL2_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_SYS_STATUS_SIZE >= 2
	// System status register
	if ((address >= C2503_IO_SYS_STATUS_ADDR) && (address < (C2503_IO_SYS_STATUS_ADDR + C2503_IO_SYS_STATUS_SIZE))) {
		WRITE_WORD(g_io_sys_status, address - C2503_IO_SYS_STATUS_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_SYS_ID_COOKIE_SIZE >= 2
	// System ID cookie
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		WRITE_WORD(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_system_write_long(unsigned address, unsigned int value) {
#if C2503_IO_SYS_CONTROL1_SIZE >= 4
	// System control register 1
	if ((address >= C2503_IO_SYS_CONTROL1_ADDR) && (address < (C2503_IO_SYS_CONTROL1_ADDR + C2503_IO_SYS_CONTROL1_SIZE))) {
		WRITE_LONG(g_io_sys_cntl1, address - C2503_IO_SYS_CONTROL1_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_SYS_CONTROL2_SIZE >= 4
	// System control register 2
	if ((address >= C2503_IO_SYS_CONTROL2_ADDR) && (address < (C2503_IO_SYS_CONTROL2_ADDR + C2503_IO_SYS_CONTROL2_SIZE))) {
		WRITE_LONG(g_io_sys_cntl2, address - C2503_IO_SYS_CONTROL2_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_SYS_STATUS_SIZE >= 4
	// System status register
	if ((address >= C2503_IO_SYS_STATUS_ADDR) && (address < (C2503_IO_SYS_STATUS_ADDR + C2503_IO_SYS_STATUS_SIZE))) {
		WRITE_LONG(g_io_sys_status, address - C2503_IO_SYS_STATUS_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_SYS_ID_COOKIE_SIZE >= 4
	// System ID cookie
	if ((address >= C2503_IO_SYS_ID_COOKIE_ADDR) && (address < (C2503_IO_SYS_ID_COOKIE_ADDR + C2503_IO_SYS_ID_COOKIE_SIZE))) {
		WRITE_LONG(g_io_sysid_cookie, address - C2503_IO_SYS_ID_COOKIE_ADDR, value);
		return true;
	}
#endif
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Counter / Timer
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_counter[C2503_IO_COUNTER_TIMER_SIZE];
unsigned char g_io_counter_cntl[C2503_IO_COUNTER_CONTROL_SIZE];

bool io_counter_read_byte(unsigned address, unsigned int *value) {
	// Counter/Timer register
	if ((address >= C2503_IO_COUNTER_TIMER_ADDR) && (address < (C2503_IO_COUNTER_TIMER_ADDR + C2503_IO_COUNTER_TIMER_SIZE))) {
		*value = READ_BYTE(g_io_counter, address - C2503_IO_COUNTER_TIMER_ADDR);
		return true;
	}
	// Counter/Timer control register
	if ((address >= C2503_IO_COUNTER_CONTROL_ADDR) && (address < (C2503_IO_COUNTER_CONTROL_ADDR + C2503_IO_COUNTER_CONTROL_SIZE))) {
		*value = READ_BYTE(g_io_counter_cntl, address - C2503_IO_COUNTER_CONTROL_ADDR);
		return true;
	}
	return false;
}

bool io_counter_read_word(unsigned address, unsigned int *value) {
#if C2503_IO_COUNTER_TIMER_SIZE >= 2
	// Counter/Timer register
	if ((address >= C2503_IO_COUNTER_TIMER_ADDR) && (address < (C2503_IO_COUNTER_TIMER_ADDR + C2503_IO_COUNTER_TIMER_SIZE))) {
		*value = READ_WORD(g_io_counter, address - C2503_IO_COUNTER_TIMER_ADDR);
		return true;
	}
#endif
#if C2503_IO_COUNTER_CONTROL_SIZE >= 2
	// Counter/Timer control register
	if ((address >= C2503_IO_COUNTER_CONTROL_ADDR) && (address < (C2503_IO_COUNTER_CONTROL_ADDR + C2503_IO_COUNTER_CONTROL_SIZE))) {
		*value = READ_WORD(g_io_counter_cntl, address - C2503_IO_COUNTER_CONTROL_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_counter_read_long(unsigned address, unsigned int *value) {
#if C2503_IO_COUNTER_TIMER_SIZE >= 4
	// Counter/Timer register
	if ((address >= C2503_IO_COUNTER_TIMER_ADDR) && (address < (C2503_IO_COUNTER_TIMER_ADDR + C2503_IO_COUNTER_TIMER_SIZE))) {
		*value = READ_LONG(g_io_counter, address - C2503_IO_COUNTER_TIMER_ADDR);
		return true;
	}
#endif
#if C2503_IO_COUNTER_CONTROL_SIZE >= 4
	// Counter/Timer control register
	if ((address >= C2503_IO_COUNTER_CONTROL_ADDR) && (address < (C2503_IO_COUNTER_CONTROL_ADDR + C2503_IO_COUNTER_CONTROL_SIZE))) {
		*value = READ_LONG(g_io_counter_cntl, address - C2503_IO_COUNTER_CONTROL_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_counter_write_byte(unsigned address, unsigned int value) {
	// Counter/Timer register
	if ((address >= C2503_IO_COUNTER_TIMER_ADDR) && (address < (C2503_IO_COUNTER_TIMER_ADDR + C2503_IO_COUNTER_TIMER_SIZE))) {
		WRITE_BYTE(g_io_counter, address - C2503_IO_COUNTER_TIMER_ADDR, value);
		return true;
	}
	// Counter/Timer control register
	if ((address >= C2503_IO_COUNTER_CONTROL_ADDR) && (address < (C2503_IO_COUNTER_CONTROL_ADDR + C2503_IO_COUNTER_CONTROL_SIZE))) {
		WRITE_BYTE(g_io_counter_cntl, address - C2503_IO_COUNTER_CONTROL_ADDR, value);
		return true;
	}
	return false;
}

bool io_counter_write_word(unsigned address, unsigned int value) {
#if C2503_IO_COUNTER_TIMER_SIZE >= 2
	// Counter/Timer register
	if ((address >= C2503_IO_COUNTER_TIMER_ADDR) && (address < (C2503_IO_COUNTER_TIMER_ADDR + C2503_IO_COUNTER_TIMER_SIZE))) {
		WRITE_WORD(g_io_counter, address - C2503_IO_COUNTER_TIMER_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_COUNTER_CONTROL_SIZE >= 2
	// Counter/Timer control register
	if ((address >= C2503_IO_COUNTER_CONTROL_ADDR) && (address < (C2503_IO_COUNTER_CONTROL_ADDR + C2503_IO_COUNTER_CONTROL_SIZE))) {
		WRITE_WORD(g_io_counter_cntl, address - C2503_IO_COUNTER_CONTROL_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_counter_write_long(unsigned address, unsigned int value) {
#if C2503_IO_COUNTER_TIMER_SIZE >= 4
	// Counter/Timer register
	if ((address >= C2503_IO_COUNTER_TIMER_ADDR) && (address < (C2503_IO_COUNTER_TIMER_ADDR + C2503_IO_COUNTER_TIMER_SIZE))) {
		WRITE_LONG(g_io_counter, address - C2503_IO_COUNTER_TIMER_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_COUNTER_CONTROL_SIZE >= 4
	// Counter/Timer control register
	if ((address >= C2503_IO_COUNTER_CONTROL_ADDR) && (address < (C2503_IO_COUNTER_CONTROL_ADDR + C2503_IO_COUNTER_CONTROL_SIZE))) {
		WRITE_LONG(g_io_counter_cntl, address - C2503_IO_COUNTER_CONTROL_ADDR, value);
		return true;
	}
#endif
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Dual UART
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_duart[C2503_IO_DUART_SIZE];

bool io_duart_read_byte(unsigned address, unsigned int *value) {
	// Dual UART
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		*value = READ_BYTE(g_io_duart, address - C2503_IO_DUART_ADDR);
		return true;
	}
	return false;
}

bool io_duart_read_word(unsigned address, unsigned int *value) {
#if C2503_IO_DUART_SIZE >= 2
	// Dual UART
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		*value = READ_WORD(g_io_duart, address - C2503_IO_DUART_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_duart_read_long(unsigned address, unsigned int *value) {
#if C2503_IO_DUART_SIZE >= 4
	// Dual UART
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		*value = READ_LONG(g_io_duart, address - C2503_IO_DUART_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_duart_write_byte(unsigned address, unsigned int value) {
	// Dual UART
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		WRITE_BYTE(g_io_duart, address - C2503_IO_DUART_ADDR, value);
		return true;
	}
	return false;
}

bool io_duart_write_word(unsigned address, unsigned int value) {
#if C2503_IO_DUART_SIZE >= 2
	// Dual UART
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		WRITE_WORD(g_io_duart, address - C2503_IO_DUART_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_duart_write_long(unsigned address, unsigned int value) {
#if C2503_IO_DUART_SIZE >= 4
	// Dual UART
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		WRITE_LONG(g_io_duart, address - C2503_IO_DUART_ADDR, value);
		return true;
	}
#endif
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Channel A: LANCE
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_chnla_lance[C2503_IO_CHANNELA_LANCE_SIZE];

bool io_channela_read_byte(unsigned address, unsigned int *value) {
	// Channel A: LANCE
	if ((address >= C2503_IO_CHANNELA_LANCE_ADDR) && (address < (C2503_IO_CHANNELA_LANCE_ADDR + C2503_IO_CHANNELA_LANCE_SIZE))) {
		*value = READ_BYTE(g_io_chnla_lance, address - C2503_IO_CHANNELA_LANCE_ADDR);
		return true;
	}
	return false;
}

bool io_channela_read_word(unsigned address, unsigned int *value) {
#if C2503_IO_CHANNELA_LANCE_SIZE >= 2
	// Channel A: LANCE
	if ((address >= C2503_IO_CHANNELA_LANCE_ADDR) && (address < (C2503_IO_CHANNELA_LANCE_ADDR + C2503_IO_CHANNELA_LANCE_SIZE))) {
		*value = READ_WORD(g_io_chnla_lance, address - C2503_IO_CHANNELA_LANCE_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_channela_read_long(unsigned address, unsigned int *value) {
#if C2503_IO_CHANNELA_LANCE_SIZE >= 4
	// Channel A: LANCE
	if ((address >= C2503_IO_CHANNELA_LANCE_ADDR) && (address < (C2503_IO_CHANNELA_LANCE_ADDR + C2503_IO_CHANNELA_LANCE_SIZE))) {
		*value = READ_LONG(g_io_chnla_lance, address - C2503_IO_CHANNELA_LANCE_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_channela_write_byte(unsigned address, unsigned int value) {
	// Channel A: LANCE
	if ((address >= C2503_IO_CHANNELA_LANCE_ADDR) && (address < (C2503_IO_CHANNELA_LANCE_ADDR + C2503_IO_CHANNELA_LANCE_SIZE))) {
		WRITE_BYTE(g_io_chnla_lance, address - C2503_IO_CHANNELA_LANCE_ADDR, value);
		return true;
	}
	return false;
}

bool io_channela_write_word(unsigned address, unsigned int value) {
#if C2503_IO_CHANNELA_LANCE_SIZE >= 2
	// Channel A: LANCE
	if ((address >= C2503_IO_CHANNELA_LANCE_ADDR) && (address < (C2503_IO_CHANNELA_LANCE_ADDR + C2503_IO_CHANNELA_LANCE_SIZE))) {
		WRITE_WORD(g_io_chnla_lance, address - C2503_IO_CHANNELA_LANCE_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_channela_write_long(unsigned address, unsigned int value) {
#if C2503_IO_CHANNELA_LANCE_SIZE >= 4
	// Channel A: LANCE
	if ((address >= C2503_IO_CHANNELA_LANCE_ADDR) && (address < (C2503_IO_CHANNELA_LANCE_ADDR + C2503_IO_CHANNELA_LANCE_SIZE))) {
		WRITE_LONG(g_io_chnla_lance, address - C2503_IO_CHANNELA_LANCE_ADDR, value);
		return true;
	}
#endif
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Channel B: LANCE / Serial
//////////////////////////////////////////////////////////////////////////////////////////////
unsigned char g_io_chnlb_lance[C2503_IO_CHANNELB_LANCE_SIZE];
unsigned char g_io_chnlb_serial[C2503_IO_CHANNELB_SERIAL_SIZE];

bool io_channelb_read_byte(unsigned address, unsigned int *value) {
	// Channel B: LANCE/serial
	if ((address >= C2503_IO_CHANNELB_LANCE_ADDR) && (address < (C2503_IO_CHANNELB_LANCE_ADDR + C2503_IO_CHANNELB_LANCE_SIZE))) {
		*value = READ_BYTE(g_io_chnlb_lance, address - C2503_IO_CHANNELB_LANCE_ADDR);
		return true;
	}
	// Channel B: serial DTR
	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		*value = READ_BYTE(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR);
		return true;
	}
	return false;
}

bool io_channelb_read_word(unsigned address, unsigned int *value) {
#if C2503_IO_CHANNELB_LANCE_SIZE >= 2
	// Channel B: LANCE/serial
	if ((address >= C2503_IO_CHANNELB_LANCE_ADDR) && (address < (C2503_IO_CHANNELB_LANCE_ADDR + C2503_IO_CHANNELB_LANCE_SIZE))) {
		*value = READ_WORD(g_io_chnlb_lance, address - C2503_IO_CHANNELB_LANCE_ADDR);
		return true;
	}
#endif
#if C2503_IO_CHANNELB_SERIAL_SIZE >= 2
	// Channel B: serial DTR
	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		*value = READ_WORD(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_channelb_read_long(unsigned address, unsigned int *value) {
#if C2503_IO_CHANNELB_LANCE_SIZE >= 4
	// Channel B: LANCE/serial
	if ((address >= C2503_IO_CHANNELB_LANCE_ADDR) && (address < (C2503_IO_CHANNELB_LANCE_ADDR + C2503_IO_CHANNELB_LANCE_SIZE))) {
		*value = READ_LONG(g_io_chnlb_lance, address - C2503_IO_CHANNELB_LANCE_ADDR);
		return true;
	}
#endif
#if C2503_IO_CHANNELB_SERIAL_SIZE >= 4
	// Channel B: serial DTR
	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		*value = READ_LONG(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR);
		return true;
	}
#endif
	return false;
}

bool io_channelb_write_byte(unsigned address, unsigned int value) {
	// Channel B: LANCE/serial
	if ((address >= C2503_IO_CHANNELB_LANCE_ADDR) && (address < (C2503_IO_CHANNELB_LANCE_ADDR + C2503_IO_CHANNELB_LANCE_SIZE))) {
		WRITE_BYTE(g_io_chnlb_lance, address - C2503_IO_CHANNELB_LANCE_ADDR, value);
		return true;
	}
	// Channel B: serial DTR
	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		WRITE_BYTE(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR, value);
		return true;
	}
	return false;
}

bool io_channelb_write_word(unsigned address, unsigned int value) {
#if C2503_IO_CHANNELB_LANCE_SIZE >= 2
	// Channel B: LANCE/serial
	if ((address >= C2503_IO_CHANNELB_LANCE_ADDR) && (address < (C2503_IO_CHANNELB_LANCE_ADDR + C2503_IO_CHANNELB_LANCE_SIZE))) {
		WRITE_WORD(g_io_chnlb_lance, address - C2503_IO_CHANNELB_LANCE_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_CHANNELB_SERIAL_SIZE >= 2
	// Channel B: serial DTR
	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		WRITE_WORD(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR, value);
		return true;
	}
#endif
	return false;
}

bool io_channelb_write_long(unsigned address, unsigned int value) {
#if C2503_IO_CHANNELB_LANCE_SIZE >= 4
	// Channel B: LANCE/serial
	if ((address >= C2503_IO_CHANNELB_LANCE_ADDR) && (address < (C2503_IO_CHANNELB_LANCE_ADDR + C2503_IO_CHANNELB_LANCE_SIZE))) {
		WRITE_LONG(g_io_chnlb_lance, address - C2503_IO_CHANNELB_LANCE_ADDR, value);
		return true;
	}
#endif
#if C2503_IO_CHANNELB_SERIAL_SIZE >= 4
	// Channel B: serial DTR
	if ((address >= C2503_IO_CHANNELB_SERIAL_ADDR) && (address < (C2503_IO_CHANNELB_SERIAL_ADDR + C2503_IO_CHANNELB_SERIAL_SIZE))) {
		WRITE_LONG(g_io_chnlb_serial, address - C2503_IO_CHANNELB_SERIAL_ADDR, value);
		return true;
	}
#endif
	return false;
}
