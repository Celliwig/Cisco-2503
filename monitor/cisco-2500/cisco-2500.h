.include "cisco-2500/scn2681.h"

# Actual location of boot ROM
.equ		bootrom_start, 0x01000000			/* Start of boot ROM */
.equ		bootrom_size, 0x80000				/* 512kB Boot ROM */

# Configure initial stack pointer
.equ		stack_ptr, 0x00000500				/* Stack pointer address on reset */
.equ		EXCEPT_RESET_SSP, stack_ptr

# Configure monitor start address
.equ		monitor_start, 0x00001000			/* Address of monitor start */
.equ		EXCEPT_RESET_PC, bootrom_start + monitor_start

# Define the boundaries for searching for modules
.equ		module_search_mem_start, monitor_start
.equ		module_search_mem_end, bootrom_start + bootrom_size
