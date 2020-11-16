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
bool		\
		// Channel A
		scn2681_channelA_rx_enabled, scn2681_channelA_mode2_selected, \
		scn2681_channelA_tx_enabled, scn2681_channelA_tx_thr_empty, scn2681_channelA_tx_tsr_empty, \
		// Channel B
		scn2681_channelB_rx_enabled, scn2681_channelB_mode2_selected, \
		scn2681_channelB_tx_enabled, scn2681_channelB_tx_thr_empty, scn2681_channelB_tx_tsr_empty;

unsigned char	\
		// Channel A Registers
		scn2681_channelA_mode1, scn2681_channelA_mode2, scn2681_channelA_status, \
		scn2681_channelA_clock_select, scn2681_channelA_command, \
		scn2681_channelA_rx, \
		scn2681_channelA_tx_thr, scn2681_channelA_tx_tsr, \
		// Channel B Registers
		scn2681_channelB_mode1, scn2681_channelB_mode2, scn2681_channelB_status, \
		scn2681_channelB_clock_select, scn2681_channelB_command, \
		scn2681_channelB_rx, \
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
		scn2681_channelA_tx_tsr_ticks, \
		// Channel B
		scn2681_channelB_tx_tsr_ticks;

int		\
		// Channel A
		scn2681_channelA_serial_device_fd = -1, \
		// Channel B
		scn2681_channelB_serial_device_fd = -1;

// Sets a file handler to use as a serial device for channel
//////////////////////////////////////////////////////////////////////////////////////////////
void io_duart_core_channelA_serial_device(int device) {
	scn2681_channelA_serial_device_fd = device;
}

void io_duart_core_channelB_serial_device(int device) {
	scn2681_channelB_serial_device_fd = device;
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
		case ChannelA_Rx:
			return scn2681_channelA_rx;
			break;
		case ChannelA_Tx_Holding:
			return scn2681_channelA_tx_thr;
			break;
		case ChannelA_Tx_Shift:
			return scn2681_channelA_tx_tsr;
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
		case ChannelB_Rx:
			return scn2681_channelB_rx;
			break;
		case ChannelB_Tx_Holding:
			return scn2681_channelB_tx_thr;
			break;
		case ChannelB_Tx_Shift:
			return scn2681_channelB_tx_tsr;
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
	scn2681_channelA_rx = 0;
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
	scn2681_channelB_rx = 0;
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
	io_duart_core_channelA_tx_disable();
	// Channel B Registers
	scn2681_channelB_mode1 = 0;
	scn2681_channelB_mode2 = 0;
	scn2681_channelB_status = 0;
	scn2681_channelB_clock_select = 0;
	scn2681_channelB_command = 0;
	io_duart_core_channelB_reset_selected_mr();
	io_duart_core_channelB_rx_disable();
	io_duart_core_channelB_tx_disable();
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
	// Channel A Registers
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
	if (!scn2681_channelA_tx_thr_empty) {
		if (scn2681_channelA_tx_tsr_empty) {
			scn2681_channelA_tx_tsr = scn2681_channelA_tx_thr;
			scn2681_channelA_tx_tsr_empty = false;
			scn2681_channelA_tx_thr_empty = true;
		}
	}
	// Channel B Registers
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
bool io_duart_read_byte(unsigned address, unsigned int *value, bool real_read) {
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
				if (scn2681_channelA_tx_thr_empty) *value = *value | SCN2681_REG_STATUS_TX_RDY;
				if (scn2681_channelA_tx_thr_empty & scn2681_channelA_tx_tsr_empty) *value = *value | SCN2681_REG_STATUS_TX_EMT;
				break;
			case SCN2681_REG_RD_BRG_TEST:			// Baud Rate Generator Test
				*value = 0;				// Not Implemented
				break;
			case SCN2681_REG_RD_RX_A:			// Channel A: RX Register
				*value = scn2681_channelA_rx;
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
				if (scn2681_channelB_tx_thr_empty) *value = *value | SCN2681_REG_STATUS_TX_RDY;
				if (scn2681_channelB_tx_thr_empty & scn2681_channelB_tx_tsr_empty) *value = *value | SCN2681_REG_STATUS_TX_EMT;
				break;
			case SCN2681_REG_RD_1X16X_TEST:			// 1X/16X Test
				*value = 0;				// Not Implemented
				break;
			case SCN2681_REG_RD_RX_B:			// Channel B: RX Register
				*value = scn2681_channelB_rx;
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
bool io_duart_write_byte(unsigned address, unsigned int value) {
	if ((address >= C2503_IO_DUART_ADDR) && (address < (C2503_IO_DUART_ADDR + C2503_IO_DUART_SIZE))) {
		switch (address - C2503_IO_DUART_ADDR) {
			case SCN2681_REG_WR_MODE_A:			// Channel A: Mode Register 1/2
				if (scn2681_channelA_mode2_selected) {
					scn2681_channelA_mode2 = value;
				} else {
					scn2681_channelA_mode1 = value;
				}
				if (!scn2681_channelA_mode2_selected) scn2681_channelA_mode2_selected = true;
				break;
			case SCN2681_REG_WR_CLOCK_SELECT_A:		// Channel A: Clock Select Register
				scn2681_channelA_clock_select = value;
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
					scn2681_channelB_mode1 = value;
				}
				if (!scn2681_channelB_mode2_selected) scn2681_channelB_mode2_selected = true;
				break;
			case SCN2681_REG_WR_CLOCK_SELECT_B:		// Channel B: Clock Select Register
				scn2681_channelB_clock_select = value;
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
