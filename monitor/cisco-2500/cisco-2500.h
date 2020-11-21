.include "cisco-2500/scn2681.h"

# Configure initial stack pointer
stack_ptr		= 0x00000500			/* Stack pointer address on reset */
EXCEPT_RESET_SSP	= stack_ptr

# Configure monitor start address
monitor_start		= 0x00001000			/* Address of monitor start */
EXCEPT_RESET_PC		= monitor_start

