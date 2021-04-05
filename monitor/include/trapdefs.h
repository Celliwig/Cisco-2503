# TRAP definitions
###########################################################################

# TRAP0
##################################
# Console Routines
##################################
.equiv	TRAP0_CONSOLE, 0x00
.equiv	TRAP0_CONOUT, 0x00						/* Write character out */
.equiv	TRAP0_CONIN, 0x01						/* Read character in (blocking) */
.equiv	TRAP0_CONIN_NB, 0x02						/* Read character in (non-blocking) */
.equiv	TRAP0_CONIN_CHK, 0x03						/* Check if character available */
##################################
# Print Routines
##################################
.equiv	TRAP0_PRINT, 0x10
.equiv	TRAP0_PRNT_STR, 0x10						/* Print string */
.equiv	TRAP0_PRNT_RSTR, 0x11						/* Print repeat string */
.equiv	TRAP0_PRNT_CSTR, 0x12						/* Print compressed string */
.equiv	TRAP0_PRNT_HEX_DIGIT, 0x13					/* Print hex digit */
.equiv	TRAP0_PRNT_HEX8, 0x14						/* Print hex byte value */
.equiv	TRAP0_PRNT_HEX16, 0x15						/* Print hex word value */
.equiv	TRAP0_PRNT_HEX32, 0x16						/* Print hex long value */
.equiv	TRAP0_PRNT_NEWLINE, 0x17					/* Print a newline */
.equiv	TRAP0_PRNT_SPACE, 0x18						/* Print a space */
.equiv	TRAP0_PRNT_VERSION, 0x19					/* Print version */
