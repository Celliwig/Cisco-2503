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
#.equ		module_search_mem_start, monitor_start
.equ		module_search_mem_start, bootrom_start
.equ		module_search_mem_end, bootrom_start + bootrom_size

# Monitor variable storage (0x500-0x6FF)
.equ		MONITOR_ADDR_TEMP0, 0x00000500			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP1, 0x00000504			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP2, 0x00000508			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP3, 0x0000050c			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP4, 0x00000510			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP5, 0x00000514			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP6, 0x00000518			/* Memory address for temp storage */
.equ		MONITOR_ADDR_TEMP7, 0x0000051c			/* Memory address for temp storage */
.equ		MONITOR_ADDR_CURRENT, 0x000005f8		/* Memory address for storing the current address */
.equ		MONITOR_ADDR_ATTRIBUTES1, 0x000005fc		/* Memory address for monitor config */
.equ		MONITOR_ADDR_STR_BUFFER, 0x00000600		/* Pointer to buffer */
.equ		MONITOR_ADDR_STR_BUFFER_SIZE, 0x100		/* Buffer length */
