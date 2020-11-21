.include "cisco-2500/scn2681.h"

# Actual location of boot ROM
bootrom_start           = 0x01000000                    /* Start of boot ROM */
bootrom_size		= 0x80000			/* 512kB Boot ROM */

# Configure initial stack pointer
stack_ptr		= 0x00000500			/* Stack pointer address on reset */
EXCEPT_RESET_SSP	= stack_ptr

# Configure monitor start address
monitor_start		= 0x00001000			/* Address of monitor start */
EXCEPT_RESET_PC		= bootrom_start + monitor_start

