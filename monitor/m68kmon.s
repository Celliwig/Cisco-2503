# ###########################################################################
# #                                                                         #
# #                                m68kMon                                  #
# #                                                                         #
# ###########################################################################
#
# 68k monitor inspired by PaulMON2
#
# Assumes RAM is not necessarily available at startup, and that an init
# module will page in RAM and set stack pointer.
#
# See also:
# 	https://www.pjrc.com/tech/8051/paulmon2.html

# Map console routines from monitor to library
###########################################################################
.equiv	console_in, scn2681_in_A
.equiv	console_in_nonblock, scn2681_in_A_nocheck
.equiv	console_in_check, scn2681_in_A_check
.equiv	console_out, scn2681_out_A
###########################################################################

# Include definitions
.include "cisco-2500/cisco-2500.h"
.include "include/stddefs.h"

# Monitor
###########################################################################
.org	monitor_start

# Startup routines
###########################################################################
# startup_cold
#################################
# This is the first section of code executed on startup before any
# hardware is initialised.
startup_cold:
	/* Basic initialisation */
	move	%sr, %d0						/* Disable external interrupts */
	or.l	#0x0700, %d0
	move	%d0, %sr

	/* Search for an init module */
	lea	bootrom_start, %a4					/* Set search start */
	lea	bootrom_start + bootrom_size, %a5			/* Set search end */
	lea	startup_cold_search_rtn, %a6				/* Set return address */

startup_cold_search:
	bra.w	module_find						/* Find next module */

startup_cold_search_rtn:
	beq.s	startup_cold_end					/* If don't find a module, just continue boot */
	cmp.b	#249, %d0						/* Check if it's an init module */
	bne.s	startup_cold_search_next				/* It's not an init module, keep searching */
	adda.l	#0x40, %a4						/* Add offset to module code */
	lea	startup_cold_end, %a6					/* Save return address in A7 */
	jmp	(%a4)							/* Execute init module */
startup_cold_search_next:
	adda.l	#0x100, %a4						/* Increment pointer */
	cmp.l	%a5, %a4						/* Check if we've reached the end */
	blt.s	startup_cold_search					/* We're not at the end so keep searching */
startup_cold_end:

# startup_warm
#################################
# This code is executed once basic system is initialised
# It's assumed that a stack is available at this point
# And Exception Vector Table exists in RAM
startup_warm:
	mov.l	#stack_ptr, %sp						/* Reset stack pointer */
	jsr	exception_handlers_setup				/* Configure EVT */

	bset.b	#MONITOR_CONFIG_PRINT, (MONITOR_ADDR_ATTRIBUTES1)	/* Printing is enabled */

	lea	module_search_mem_start, %a4				/* Set search start */
	mov.b	#253, %d1						/* Search for startup application */
	bsr.w	module_search
	cmp.b	#0, %d0
	beq.s	startup_warm_end					/* No module found, so finish */

	adda.l	#0x40, %a4						/* Add offset to module code */
	jsr	(%a4)							/* Execute startup module */
startup_warm_end:

# startup_final
#################################
# Final section of startup, executes menu system
startup_final:
	jsr	print_newline
	jsr	print_newline

	lea	str_logon1, %a0						/* Print greeting */
	jsr	print_cstr
	jsr	get_version						/* Get version number */
	jsr	print_version
	jsr	print_newline

#	lea	str_logon2						/* Print documentation notes */
#	jsr	print_cstr

	jsr	module_list_commands					/* Print included commands */

	jmp	menu_main						/* Enter main menu */

# get_version
#################################
#  Returns the version number of the monitor
#	Out:	D1 = Minor version number [16] | Major version number [16]
get_version:
	mov.l	#M68KMON_VERSION, %d1
	rts

# Exception handlers
###########################################################################
# exception_handlers_setup
#################################
#  Configure EVT vectors
exception_handlers_setup:
	# Configure base exception handler
	lea	exception_handler_unknown, %a0				/* Use unknown exception handler */
	mov.l	#EVT_ACCESS_ERROR, %a1					/* First address in EVT after reset SP/PC */
	mov.w	#0xfd, %d0						/* Number of EVT entries to update */
exception_handlers_setup_base_loop:
	mov.l	%a0, (%a1)+						/* Copy exception handler address to EVT */
	dbf	%d0, exception_handlers_setup_base_loop			/* Loop over EVT table */

	lea	exception_handler_address, %a0
	mov.l	%a0, (EVT_ADDRESS_ERROR)

	lea	exception_handler_bus, %a0
	mov.l	%a0, (EVT_ACCESS_ERROR)

	lea	exception_handler_interrupt, %a0
	mov.l	%a0, (EVT_INTERRUPT_LEVEL1)
	mov.l	%a0, (EVT_INTERRUPT_LEVEL2)
	mov.l	%a0, (EVT_INTERRUPT_LEVEL3)
	mov.l	%a0, (EVT_INTERRUPT_LEVEL4)
	mov.l	%a0, (EVT_INTERRUPT_LEVEL5)
	mov.l	%a0, (EVT_INTERRUPT_LEVEL6)
	mov.l	%a0, (EVT_INTERRUPT_LEVEL7)

	lea	exception_handler_zero, %a0
	mov.l	%a0, (EVT_DIVIDE_BY_ZERO)

	lea	trap_0, %a0
	mov.l	%a0, (EVT_TRAP0_INSTRUCTION)

	rts

# Basic exception handlers
#################################
exception_handler_address:
	lea	str_exception_address, %a1
	jmp	exception_handler_base
exception_handler_bus:
	lea	str_exception_bus, %a1
	jmp	exception_handler_base
exception_handler_interrupt:
	lea	str_exception_interrupt, %a1
	jmp	exception_handler_base
exception_handler_zero:
	lea	str_exception_zero, %a1
	jmp	exception_handler_base
exception_handler_unknown:
	lea	str_exception_unknown, %a1
# exception_handler_base
#################################
#  Print exception message and reboot
#  (Assumes stacks working!!!)
#	In:	A1 - Exception message
exception_handler_base:
	jsr	print_newline

	# Print top border
	mov.b	#M68KMON_EXCEPTION_BORDER_CHAR, %d0
	mov.w	#M68KMON_EXCEPTION_BORDER_WIDTH, %d1
	jsr	print_char_n
	jsr	print_newline

	# Print banner
	mov.b	#M68KMON_EXCEPTION_BORDER_CHAR, %d0
	jsr	console_out
	mov.b	#' ', %d0
	mov.w	#((M68KMON_EXCEPTION_BORDER_WIDTH - 2 - str_exception_ga_size) / 2), %d1
	jsr	print_char_n
	lea	str_exception_ga, %a0
	jsr	print_str_simple
	mov.b	#' ', %d0
	# Don't assume string is even number of characters
	mov.w	#((M68KMON_EXCEPTION_BORDER_WIDTH - 2 - str_exception_ga_size) - ((M68KMON_EXCEPTION_BORDER_WIDTH - str_exception_ga_size - 2) / 2)), %d1
	jsr	print_char_n
	mov.b	#M68KMON_EXCEPTION_BORDER_CHAR, %d0
	jsr	console_out
	jsr	print_newline

	# Print message
	mov.b	#M68KMON_EXCEPTION_BORDER_CHAR, %d0
	jsr	console_out
	mov.b	#' ', %d0
	jsr	console_out
	mov.l	%a1, %a0
	jsr	print_str_simple
	mov.l	%a1, %a0
	jsr	string_length
	mov.w	#(M68KMON_EXCEPTION_BORDER_WIDTH - 3), %d1
	sub.w	%d0, %d1
	mov.b	#' ', %d0
	jsr	print_char_n
	mov.b	#M68KMON_EXCEPTION_BORDER_CHAR, %d0
	jsr	console_out
	jsr	print_newline

	# Print bottom border
	mov.b	#M68KMON_EXCEPTION_BORDER_CHAR, %d0
	mov.w	#M68KMON_EXCEPTION_BORDER_WIDTH, %d1
	jsr	print_char_n
	jsr	print_newline

	jmp	startup_cold

# Module routines
###########################################################################
# module_find
#################################
#  Finds the next header in the external memory. (Has to be able to work without stack)
#	In:	A4 = point to start of search
#		A5 = point to end of search
#		A6 = Return address
#	Out:	A4 = location of next module
#		D0 = module type, or unset if no module found
module_find:
	mov.l	%a4, %d0						/* Make sure we are on a boundary */
	andi.l	#0xffffff00, %d0
	mov.l	%d0, %a4
module_find_loop:
	mov.l	%a4, %a1						/* Copy address */
	cmp.b	#0xA5, (%a1)+						/* Check first byte */
	bne.s	module_find_next					/* If this doesn't match, check next range */
	cmp.b	#0xE5, (%a1)+						/* Check second byte */
	bne.s	module_find_next					/* If this doesn't match, check next range */
	cmp.b	#0xE0, (%a1)+						/* Check third byte */
	bne.s	module_find_next					/* If this doesn't match, check next range */
	cmp.b	#0xA5, (%a1)+						/* Check fourth byte */
	bne.s	module_find_next					/* If this doesn't match, check next range */
	mov.b	(%a1), %d0						/* Get module type */
	jmp	(%a6)							/* Resume execution from where we left off */
module_find_next:
	adda.l	#0x100, %a4						/* Increment address pointer */
	cmp.l	%a5, %a4						/* Check if we've reached the end */
	blt.s	module_find_loop
module_find_end:
	eor.l	%d0, %d0						/* Clear D0 to indicate no module */
	jmp	(%a6)							/* Resume execution from where we left off */

# module_search
#################################
# Finds the next header in the external memory.
#	In:	A4 = Address to start searching from
#		D1 = Module type to search for
#	Out:	A4 = location of next module
#		D0 = Found module type (cleared on failure)
module_search:
	lea	module_search_mem_end, %a5				/* Set search end */
	lea	module_search_reenter, %a6				/* Set return address */
module_search_next:
	cmp.l	%a5, %a4						/* Check if we've reached the end */
	bge.s	module_search_failed					/* We've reached the end, so we've failed to find a module */
	jmp	module_find
module_search_reenter:
	beq.s	module_search_failed					/* No module, exit */
	cmp.b	%d1, %d0						/* Check if module type we're looking for */
	beq.s	module_search_end					/* If they're the same, return */
	adda.l	#0x100, %a4						/* Increment address pointer */
	bra.s	module_search_next					/* Continue search */
module_search_failed:
	eor.l	%d0, %d0						/* Clear D0 as we've failed */
module_search_end:
	and.b	%d0, %d0						/* Set status bits appropriately */
	rts

# String routines
###########################################################################
# char_2_upper
#################################
#  Converts ASCII letter to uppercase if necessary
#	In:	D0 = ASCII character
#	Out:	D0 = ASCII character
char_2_upper:
	cmp.b	#97, %d0						/* 'a' */
	blt.s	char_2_upper_end
	cmp.b	#123, %d0						/* 'z' + 1 */
	bgt.s	char_2_upper_end
	add.b	#224, %d0						/* Addition wraps */
char_2_upper_end:
	rts

# char_2_hex
#################################
#  Converts (if possible) a character (A-F/0-9) to hex value
#    Must be uppercase!!!
#	In:	D0 = ASCII character
#	Out:	D0 = Hex value
#		Carry set if value valid, not otherwise
char_2_hex:
	sub.b	#'0', %d0						/* Remove initial offset for 0-9 */
	bcs.s	char_2_hex_error					/* < '0', error */
	cmp.b	#10, %d0						/* Check if less than 10 */
	blt.s	char_2_hex_finish					/* If so, then finish */
	sub.b	#7, %d0							/* Remove secondary offset for A-F */
	cmp.b	#0x10, %d0						/* Check if less than 16 */
	blt.s	char_2_hex_finish					/* If so, then finish */
char_2_hex_error:
	andi.b	#0xfe, %ccr						/* Invalid value */
	rts
char_2_hex_finish:
	ori.b	#0x01, %ccr						/* Valid value */
	rts

# string_2_hex32
#################################
#  Reads 8 character (hex) digits and returns the long value
#	In:	A0 = Pointer to string buffer
#	Out:	D1 = Value
#		Carry flag set if value valid
string_2_hex32:
	eor.l	%d1, %d1						/* Clear register */
string_2_hex32_initialised:
	jsr	string_2_hex16_initialised				/* Read first word */
	bcc.s	string_2_hex_finish					/* Exit on error */
	bra.s	string_2_hex16_initialised				/* Jump over initialisation */
# string_2_hex16
#################################
#  Reads 4 character (hex) digits and returns the word value
#	In:	A0 = Pointer to string buffer
#	Out:	D1 = Value
#		Carry flag set if value valid
string_2_hex16:
	eor.l	%d1, %d1						/* Clear register */
string_2_hex16_initialised:
	jsr	string_2_hex8_initialised				/* Read first byte */
	bcc.s	string_2_hex_finish					/* Exit on error */
	bra.s	string_2_hex8_initialised				/* Jump over initialisation */
# string_2_hex8
#################################
#  Reads 2 character (hex) digits and returns the byte value
#	In:	A0 = Pointer to string buffer
#	Out:	D1 = Value
#		Carry flag set if value valid
string_2_hex8:
	eor.l	%d1, %d1						/* Clear register */
string_2_hex8_initialised:
	jsr	string_2_hex						/* Read first character */
	bcc.s	string_2_hex_finish					/* Exit on error */
# string_2_hex
#################################
#  Reads a character (hex) digit and returns the byte value
#	In:	A0 = Pointer to string buffer
#		D1 = Return for value (expects to be cleared)
#	Out:	D1 = Value
#		Carry flag set if value valid
string_2_hex:
	lsl.l	#4, %d1							/* Move existing value by a nibble */
	mov.b	(%a0)+, %d0
	jsr	char_2_upper						/* Ensure character uppercase */
	jsr	char_2_hex						/* Convert character to value */
	bcc.s	string_2_hex_finish					/* Exit on error */
	or.b	%d0, %d1						/* Copy value from D0 to D1 */
	ori.b	#0x01, %ccr						/* Valid value */
string_2_hex_finish:
	rts

# string_hex_2_dec
#################################
#  Convert a value to decimal, store result in string buffer
#	In:	A0 = Pointer to string buffer
#		D0 = Value
string_hex_2_dec:
	movm.l	%d1-%d7, -(%sp)						/* Save registers */
	mov.l	%d0, %d7						/* Copy value */
	bpl.s	string_hex_2_dec_init					/* Check if an '-' is needed */
	neg.l	%d7							/* Change to positive */
	bmi.s	string_hex_2_dec_zero					/* Value is zero */
	mov.b	#'-', (%a0)+						/* Add '-' symbol */
string_hex_2_dec_init:
	clr.w	%d4							/* Init zero suppression */
	movq.l	#10, %d6						/* Digit count */
string_hex_2_dec_calc_power:
	movq.l	#1, %d2							/* Initial power value */
	mov.l	%d6, %d1						/* Power value count */
	subq.l	#1, %d1
	beq.s	string_hex_2_dec_calc_digit				/* Continue at power of 0 */
string_hex_2_dec_calc_power_loop:
	/* Basic 68000 only supports word sized multiplication, product is 32 bits however */
	/* So this requires convoluted calculation */
	/* D2 = Upper 16 bits */
	/* D3 = Lower 16 bits */
	mov.w	%d2, %d3						/* Copy current value */
	mulu.w	#10, %d3						/* Multiply current lower value by 10 */
	swap.w	%d3							/* Swap to access upper 16 bits of result */
	swap.w	%d2							/* Swap to access current upper 16 bits */
	mulu.w	#10, %d2						/* Multiply upper value by 10 */
	add.w	%d3, %d2						/* Add upper 16 bit result to current */
	swap.w	%d2							/* Swap registers back around */
	swap.w	%d3
	mov.w	%d3, %d2						/* Combine lower result and upper result */
	subq.l	#1, %d1							/* Decrement power count */
	bne	string_hex_2_dec_calc_power_loop
string_hex_2_dec_calc_digit:
	clr.l	%d0							/* Clear digit value */
string_hex_2_dec_calc_digit_loop:
	cmp.l	%d2, %d7						/* Check if value is less than current power */
	blt.s	string_hex_2_dec_check_val				/* If so, process value */
	addq.l	#1, %d0							/* Increment digit value */
	sub.l	%d2, %d7						/* Subtract current power from value */
	bra.s	string_hex_2_dec_calc_digit_loop			/* Loop */
string_hex_2_dec_check_val:
	tst.b	%d0							/* Check if zero */
	bne.s	string_hex_2_dec_buffer_put
	tst.w	%d4							/* Check to suppress leading zeros */
	beq.s	string_hex_2_dec_next_digit
string_hex_2_dec_buffer_put:
	addi.b	#'0', %d0						/* Convert value to ASCII character */
	mov.b	%d0, (%a0)+						/* Place character in buffer */
	mov.b	%d0, %d4						/* Disable zero suppression */
string_hex_2_dec_next_digit:
	subq.l	#1, %d6							/* Next digit */
	bne.s	string_hex_2_dec_calc_power
	tst.w	%d4							/* Anything printed? */
	bne.s	string_hex_2_dec_finish
string_hex_2_dec_zero:
	mov.b	#'0', (%a0)+						/* Print at least a zero */
string_hex_2_dec_finish:
	mov.b	#0, (%a0)						/* Add string terminator */
	movm.l	(%sp)+, %d1-%d7						/* Restore registers */
	rts

# string_length
#################################
#  Calculates the length of the string
#	In:	A0 = Pointer to string
#	Out:	D0 = String length
string_length:
	eor.l	%d0, %d0						/* Clear D0 */
string_length_loop:
	mov.b	(%a0)+, %d1						/* Copy, check whether character is zero, increment pointer */
	beq.s	string_length_end					/* EOL */
	addi.w	#1, %d0							/* Increment character count */
	btst	#7, %d1							/* See if we have a multi-part string */
	bne.s	string_length_end
	bra.s	string_length_loop					/* Loop */
string_length_end:
	rts

# Print routines
###########################################################################
# monitor_out
#################################
#  Print a character to the terminal (if printing is enabled)
monitor_out:
	btst.b	#MONITOR_CONFIG_PRINT, (MONITOR_ADDR_ATTRIBUTES1)
	beq.s	monitor_out_finish
	jsr	console_out
monitor_out_finish:
	rts

# print_spacex2
#################################
#  Print double space
print_spacex2:
	jsr	print_space
# print_space
#################################
#  Print a space
print_space:
	mov.b	#' ', %d0
	jmp	console_out

# print_char_n
#################################
#  Print n characters
#	In:	D0 = Character to print
#		D1 = Number of spaces
print_char_n:
	subi.w	#1, %d1							/* For dbf loop to make sense */
print_char_n_loop:
	jsr	console_out
	dbf	%d1, print_char_n_loop
print_char_n_exit:
	rts

# print_dash
#################################
#  Print '-'
print_dash:
	mov.b	#'-', %d0
	jmp	console_out
# print_dash_spaces
#################################
#  Print ' - '
print_dash_spaces:
	jsr	print_space
	jsr	print_dash
	jmp	print_space

# print_colon_space
#################################
#  Print ': '
print_colon_space:
	mov.b	#':', %d0
	jsr	console_out
	jmp	print_space

# print_newlinex2
#################################
#  Print 2 new lines
print_newlinex2:
	jsr	print_newline
# print_newline
#################################
#  Print a new line
print_newline:
	mov.b	#ascii_carriage_return, %d0
	jsr	console_out
	mov.b	#ascii_linefeed, %d0
	jsr	console_out
	rts

# print_hex32
#################################
#  Print 32 bit number as hex
#	In:	D3 = 32-bit Integer
print_hex32:
	mov.l	%d3, %d2						/* Copy long */
	swap	%d2							/* Swap top and bottom 16 bits */
	jsr	print_hex16						/* Print top 2 bytes */
	mov.l	%d3, %d2						/* Copy long */
# print_hex16
#################################
#  Print 16 bit number as hex
#	In:	D2 = 16-bit Integer
print_hex16:
	mov.w	%d2, %d1						/* Copy word */
	lsr	#8, %d1							/* Shift top byte down */
	jsr	print_hex8						/* Print top byte */
	mov.w	%d2, %d1						/* Copy word */
# print_hex8
#################################
#  Print 8 bit number as hex
# 	In:	D1 = 8-bit Integer
print_hex8:
	mov.b	%d1, %d0						/* Copy byte */
	lsr	#4, %d0							/* Shift top nibble down */
	jsr	print_hex_digit
	mov.b	%d1, %d0						/* Copy byte */
print_hex_digit:
	and.b	#0xf, %d0						/* Extract bottom nibble */
	add.b	#'0', %d0						/* Add base character */
	cmp.b	#'9', %d0						/* Check whether it's in range 0-9 */
	ble.s	print_hex_digit_out
	add.b	#7, %d0
print_hex_digit_out:
	jsr	console_out						/* Print character */
	rts

# print_version
#################################
#  Prints the version number
#	In:	D1 = Version number
print_version:
	mov.b	#'v', %d0
	jsr	console_out

	# Major number
	eor.l	%d0, %d0						/* Clear register */
	swap	%d1							/* Get upper word */
	mov.w	%d1, %d0
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple

	mov.b	#'.', %d0
	jsr	console_out

	# Minor number
	eor.l	%d0, %d0						/* Clear register */
	swap	%d1							/* Get lower word */
	mov.w	%d1, %d0
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple

	rts

# print_str_simple
#################################
#  Prints a null terminated string.
#  Retains the ability ability to print consecutive strings.
#	In:	A0 = Pointer to string
print_str_simple:
	mov.b	(%a0)+, %d0						/* Get next character and increment pointer */
	beq.b	print_str_simple_end					/* Check whether NULL character */
	jsr	console_out						/* Print character */
	bra.s	print_str_simple					/* Loop */
print_str_simple_end:
	rts

print_str_simple_space:
	jsr	print_str_simple
	jmp	print_space

print_str_simple_newline:
	jsr	print_str_simple
	jmp	print_newline

# print_str_repeat
#################################
#  Prints a string of repeated character.
#  Format:	n(0) = Num. char. to print, n(1) = Character to print
#		Repeats until null character
#	In:	A0 = Pointer to string
print_str_repeat:
	eor.l	%d1, %d1						/* Clear D1 for dbeq later */
	mov.b	(%a0)+, %d1						/* Get character count and increment pointer */
	beq.b	print_str_repeat_end					/* Check for EOL */
	subi.w	#1, %d1							/* -1 for dbf */
	mov.b	(%a0)+, %d0						/* Get character and increment pointer */
print_str_repeat_loop:
	jsr	console_out						/* Print character */
	dbf	%d1, print_str_repeat_loop
	bra.s	print_str_repeat
print_str_repeat_end:
	rts

# print_str
#################################
#  Prints a null terminated string.
#  Retains the ability ability to print consecutive strings.
#	In:	A0 = Pointer to string
print_str:
	mov.b	(%a0), %d0						/* Get next character */
	beq.s	print_str_end						/* Check for end of string */
	and.b	#0x7f, %d0						/* Strip stop bit */
	jsr	console_out						/* Print character */
	btst	#7, (%a0)+						/* Test msb, increment pointer */
	bne.s	print_str_end						/* If msb set, stop */
	bra.s	print_str						/* Loop */
print_str_end:
	rts

# print_cstr
#################################
#  Prints compressed strings. A dictionary of 128 words is stored in 4
#  bit packed binary format. When it finds a byte in a string with the
#  high bit set, it prints the word from the dictionary. A few bytes
#  have special functions and everything else prints as if it were an
#  ordinary string.
#
#  Special codes for pcstr:
#	0 = end of string
#	13 = CR/LF
#	14 = CR/LF and end of string
#	31 = next word code should be capitalized
#
#	In:	A0 = Pointer to compressed string
print_cstr:
	eor.l	%d2, %d2						/* Clear D1, used for state information */
	bset	#1, %d2							/* If unset, prints a space in front of compressed word */
	bset	#5, %d2							/* If unset, prints word with capitalised first letter */
print_cstr_next:
	mov.b	(%a0)+, %d0						/* Get next character, increment pointer */
	beq.s	print_cstr_end						/* Check whether NULL character */
	btst	#7, %d0							/* Test msb */
	beq.s	print_cstr_check_13
	mov.b	%d0, %d1
	jsr	dictionary_print_word					/* It's a dictionary word, so print it */
	bra.s	print_cstr_next
print_cstr_check_13:
	cmp.b	#13, %d0						/* Check for control code */
	bne.s	print_cstr_check_14
	jsr	print_newline
	bset	#1, %d2							/* No automatic space */
	bra.s	print_cstr_next						/* Loop for next character */
print_cstr_check_14:
	cmp.b	#14, %d0						/* Check for control code */
	bne.s	print_cstr_check_31
	jsr	print_newline
	bra.s	print_cstr_end						/* Finished */
print_cstr_check_31:
	cmp.b	#31, %d0						/* Check for control code */
	bne.s	print_cstr_char
	bclr	#5, %d2							/* Capitalise the next word */
	bra.s	print_cstr_next						/* Loop for next character */
print_cstr_char:
	bclr	#1, %d2							/* Ensure a space is printed in front of the next compressed word */
	jsr	console_out						/* Print character */
	bra.s	print_cstr_next						/* Loop to next character */
print_cstr_end:
	rts

# dictionary_print_word
#################################
#  Prints a selected word from the built-in dictionary
#	In:	D1 = Selected word index
#		D2 = Control byte
#			bit 1 - 0 = Print space / 1 = Don't print space
#			bit 5 - 0 = Capitalise first letter / 1 = Don't capitalise first letter
#			bit 7 - Used to indicate top/bottom nibble
dictionary_print_word:
	andi.b	#0x7f, %d1						/* Clear msb of word index */
	bclr.l	#1, %d2							/* Check whether to print space */
	bne.s	dictionary_print_word_search_setup
	jsr	print_space						/* Print space */
dictionary_print_word_search_setup:
	lea	common_words, %a1
	bset.l	#7, %d2							/* Make sure we get the bottom nibble to start */
	and.b	%d1, %d1						/* Update flags */
dictionary_print_word_search_loop:
	beq.s	dictionary_print_word_selected_loop			/* Found the word, so print it */
dictionary_print_word_nibble_loop:
	jsr	dictionary_get_next_nibble				/* Scan through nibbles */
	bne.s	dictionary_print_word_nibble_loop			/* Searching for the end of a word (D0 = 0) */
	subq.b	#1, %d1							/* Decrement word index */
	bra.s	dictionary_print_word_search_loop			/* Search again */
dictionary_print_word_selected_loop:
	jsr	dictionary_get_next_nibble				/* Get next compressed character*/
	beq.s	dictionary_print_word_end				/* End of the word, so finish */
	cmp.b	#0x0f, %d0						/* Check whether to select alternate letters */
	beq.s	dictionary_print_word_select_unloved
dictionary_print_word_select_loved:
	lea	dictionary_loved_letters, %a2				/* Get pointer to loved letters */
	bra.s	dictionary_print_word_get_char
dictionary_print_word_select_unloved:
	jsr	dictionary_get_next_nibble				/* Get the next character */
	lea	dictionary_unloved_letters, %a2				/* Get pointer to unloved letters */
dictionary_print_word_get_char:
	subq.b	#1, %d0							/* Decrement index */
	add.l	%d0, %a2						/* Create pointer to letter */
	mov.b	(%a2), %d0						/* Get letter */
	bset	#5, %d2							/* Test whether to capitalise letter, reset flag */
	bne.s	dictionary_print_word_print_char
	jsr	char_2_upper						/* Capitalise letter */
dictionary_print_word_print_char:
	jsr	console_out						/* Print character */
	bra.s	dictionary_print_word_selected_loop			/* Loop through additional characters */
dictionary_print_word_end:
	rts

dictionary_loved_letters:	.ascii	"etarnisolumpdc"		/* 14 most commonly used letters */
dictionary_unloved_letters:	.ascii	"hfwgybxvkqjz"			/* 12 least commonly used letters */

# dictionary_get_next_nibble
#################################
#  Returns the selected nibble from the memory location
#  Prints a selected word from the built-in dictionary
#	In:	A1 = Pointer to memory location
#		D2 = Control byte
#			bit 7 - Used to indicate top/bottom nibble
#	Out:	D0 = Nibble value
dictionary_get_next_nibble:
	eor.l	%d0, %d0						/* Clear D0, will want to do long addition to an address later */
	mov.b	(%a1), %d0
	bchg.l	#7, %d2							/* Test whether we want top or bottom nibble, and toggle */
	bne.s	dictionary_get_next_nibble_bottom			/* We want the top nibble */
dictionary_get_next_nibble_top:
	adda.l	#1, %a1							/* Increment pointer */
	lsr.b	#4, %d0							/* Top nibble shifted to the bottom */
dictionary_get_next_nibble_bottom:
	and.b	#0x0f, %d0
	rts

# print_abort
#################################
#  Print the abort string
print_abort:
	lea	str_abort, %a0
	jmp	print_cstr

# print_registers
#################################
#  Print out the contents of all registers (without altering them!)
command_print_registers:
print_registers:
	# Save stack pointer in memory
	mov.l	%sp, MONITOR_ADDR_TEMP1					/* Save stack pointer */

	# Save a copy to restore later (in case this is used in an ISR)
	# Save special registers
	move	%sr, -(%sp)						/* Save status register */

	# Save address registers
	mov.l	%a6, -(%sp)						/* Save A6 pointer address */
	mov.l	%a5, -(%sp)						/* Save A5 pointer address */
	mov.l	%a4, -(%sp)						/* Save A4 pointer address */
	mov.l	%a3, -(%sp)						/* Save A3 pointer address */
	mov.l	%a2, -(%sp)						/* Save A2 pointer address */
	mov.l	%a1, -(%sp)						/* Save A1 pointer address */
	mov.l	%a0, -(%sp)						/* Save A0 pointer address */

	# Save data registers
	mov.l	%d7, -(%sp)						/* Save D7 pointer address */
	mov.l	%d6, -(%sp)						/* Save D6 pointer address */
	mov.l	%d5, -(%sp)						/* Save D5 pointer address */
	mov.l	%d4, -(%sp)						/* Save D4 pointer address */
	mov.l	%d3, -(%sp)						/* Save D3 pointer address */
	mov.l	%d2, -(%sp)						/* Save D2 pointer address */
	mov.l	%d1, -(%sp)						/* Save D1 pointer address */
	mov.l	%d0, -(%sp)						/* Save D0 pointer address */

	# Copy for display
	# Save special registers
	move	%sr, -(%sp)						/* Save status register */

	# Save address registers
	mov.l	%a6, -(%sp)						/* Save A6 pointer address */
	mov.l	%a5, -(%sp)						/* Save A5 pointer address */
	mov.l	%a4, -(%sp)						/* Save A4 pointer address */
	mov.l	%a3, -(%sp)						/* Save A3 pointer address */
	mov.l	%a2, -(%sp)						/* Save A2 pointer address */
	mov.l	%a1, -(%sp)						/* Save A1 pointer address */
	mov.l	%a0, -(%sp)						/* Save A0 pointer address */

	# Save data registers
	mov.l	%d7, -(%sp)						/* Save D7 pointer address */
	mov.l	%d6, -(%sp)						/* Save D6 pointer address */
	mov.l	%d5, -(%sp)						/* Save D5 pointer address */
	mov.l	%d4, -(%sp)						/* Save D4 pointer address */
	mov.l	%d3, -(%sp)						/* Save D3 pointer address */
	mov.l	%d2, -(%sp)						/* Save D2 pointer address */
	mov.l	%d1, -(%sp)						/* Save D1 pointer address */
	mov.l	%d0, -(%sp)						/* Save D0 pointer address */

	jsr	print_newline

	# Print data registers
	mov.w	#0, %d4
	jsr	print_newline
	mov.b	#' ', %d0
	mov.w	#2, %d1
	jsr	print_char_n
print_registers_dreg_loop:
	mov.b	#'D', %d0						/* Print 'D' */
	jsr	console_out
	mov.b	%d4, %d0						/* Register count */
	jsr	print_hex_digit
	jsr	print_colon_space					/* Print ': ' */
	mov.l	(%sp)+, %d3						/* Pop value */
	jsr	print_hex32
	mov.b	#' ', %d0
	mov.w	#3, %d1							/* Print spaces */
	jsr	print_char_n
	add.b	#1, %d4							/* Increment count */
	cmp.b	#8, %d4
	blt.s	print_registers_dreg_loop

	# Print address registers
	mov.w	#0, %d4
	jsr	print_newline
	mov.b	#' ', %d0
	mov.w	#2, %d1
	jsr	print_char_n
print_registers_areg_loop:
	mov.b	#'A', %d0						/* Print 'D' */
	jsr	console_out
	mov.b	%d4, %d0						/* Register count */
	jsr	print_hex_digit
	jsr	print_colon_space					/* Print ': ' */
	mov.l	(%sp)+, %d3						/* Pop value */
	jsr	print_hex32
	mov.b	#' ', %d0
	mov.w	#3, %d1							/* Print spaces */
	jsr	print_char_n
	add.b	#1, %d4							/* Increment count */
	cmp.b	#7, %d4
	blt.s	print_registers_areg_loop

	# Print additional registers
	jsr	print_newline
	mov.b	#' ', %d0
	mov.w	#2, %d1
	jsr	print_char_n
	lea	str_reg_sr, %a0						/* Print 'SR: ' */
	jsr	print_str
	mov.w	(%sp)+, %d3						/* Pop value */
	jsr	print_hex16
	mov.b	#' ', %d0
	mov.w	#3, %d1							/* Print spaces */
	jsr	print_char_n
	lea	str_reg_sp, %a0						/* Print 'SP: ' */
	jsr	print_str
	mov.l	MONITOR_ADDR_TEMP1, %d3					/* Pop value */
	jsr	print_hex32
	jsr	print_newline

	# Restore register contents
	# Restore data registers
	mov.l	(%sp)+, %d0						/* Restore D0 pointer address */
	mov.l	(%sp)+, %d1						/* Restore D1 pointer address */
	mov.l	(%sp)+, %d2						/* Restore D2 pointer address */
	mov.l	(%sp)+, %d3						/* Restore D3 pointer address */
	mov.l	(%sp)+, %d4						/* Restore D4 pointer address */
	mov.l	(%sp)+, %d5						/* Restore D5 pointer address */
	mov.l	(%sp)+, %d6						/* Restore D6 pointer address */
	mov.l	(%sp)+, %d7						/* Restore D7 pointer address */

	# Restore address registers
	mov.l	(%sp)+, %a0						/* Restore A0 pointer address */
	mov.l	(%sp)+, %a1						/* Restore A1 pointer address */
	mov.l	(%sp)+, %a2						/* Restore A2 pointer address */
	mov.l	(%sp)+, %a3						/* Restore A3 pointer address */
	mov.l	(%sp)+, %a4						/* Restore A4 pointer address */
	mov.l	(%sp)+, %a5						/* Restore A5 pointer address */
	mov.l	(%sp)+, %a6						/* Restore A6 pointer address */

	# Restore special registers
	move	(%sp)+, %sr						/* Save status register */

	rts

# Input routines
###########################################################################
# input_character_filter
#################################
#  Routine replaces character sequences for up/down/left/right & PageUp/PageDown
#  with values noted below.
#	Return Value	Key		Escape Sequence
#	11 (^K)		Up		1B 5B 41
#	10 (^J)		Down		1B 5B 42
#	21 (^U)		Right		1B 5B 43
#	8 (^H)		Left		1B 5B 44
#	25 (^Y)		PageUp		1B 5B 35 7E
#	26 (^Z)		PageDown	1B 5B 36 7E
#
#	Out:	D0 = ASCII character code
input_character_filter:
	jsr	console_in						/* Get character if there is one */
input_character_filter_end:
	rts

*;input_character_filter:
*;	jnb	ri, cinf1
*;	lcall	cin
*;	cjne	a, #esc_char, cinf_end
*;	;if esc was already in sbuf, just ignore it
*;cinf1:
*;	lcall	cin
*;	cjne	a, #esc_char, cinf_end
*;cinf2:
*;	acall	cinf_wait
*;	jb	ri, cinf4
*;	mov	a, #esc_char
*;	ret			;an ordinary ESC
*;
*;cinf4:	;if we get here, it's a control code, since a character
*;	;was received shortly after receiving an ESC character
*;	lcall	cin
*;	cjne	a, #'[', cinf_consume
*;	acall	cinf_wait
*;	jnb	ri, input_character_filter
*;	lcall	cin
*;cinf5a:
*;	cjne	a, #'A', cinf5b
*;	mov	a, #11
*;	ret
*;cinf5b:
*;	cjne	a, #'B', cinf5c
*;	mov	a, #10
*;	ret
*;cinf5c:
*;	cjne	a, #'C', cinf5d
*;	mov	a, #21
*;	ret
*;cinf5d:
*;	cjne	a, #'D', cinf5e
*;	mov	a, #8
*;	ret
*;cinf5e:
*;	cjne	a, #0x35, cinf5f
*;	sjmp	cinf8
*;cinf5f:
*;	cjne	a, #0x36, cinf5g
*;	sjmp	cinf8
*;cinf5g:
*;	sjmp	cinf_consume		;unknown escape sequence
*;
*;cinf8:	;when we get here, we've got the sequence for pageup/pagedown
*;	;but there's one more incoming byte to check...
*;	push	acc
*;	acall	cinf_wait
*;	jnb	ri, cinf_restart
*;	lcall	cin
*;	cjne	a, #0x7E, cinf_notpg
*;	pop	acc
*;	add	a, #228
*;cinf_end:
*;	ret
*;cinf_restart:
*;	pop	acc
*;	sjmp	input_character_filter
*;cinf_notpg:
*;	pop	acc
*;;unrecognized escape... eat up everything that's left coming in
*;;quickly, then begin looking again
*;cinf_consume:
*;	acall	cinf_wait
*;	jnb	ri, input_character_filter
*;	lcall	cin
*;	cjne	a, #esc_char, cinf_consume
*;	sjmp	cinf2
*;
*;;this thing waits for a character to be received for approx
*;;4 character transmit time periods.  It returns immedately
*;;or after the entire wait time.	 It does not remove the character
*;;from the buffer, so ri should be checked to see if something
*;;actually did show up while it was waiting
*;	.equ	char_delay, 4		;number of char xmit times to wait
*;cinf_wait:
*;	mov	a, r2
*;	push	acc
*;	mov	r2, #char_delay*5
*;cinfw2:
*;	mov	a, th0
*;cinfw3:
*;	jb	ri, cinfw4
*;	inc	a
*;	jnz	cinfw3
*;	djnz	r2, cinfw2
*;cinfw4:
*;	pop	acc
*;	mov	r2, a
*;	ret

# input_hex8_preloaded
#################################
#  Routine to enter up to 2 digit hexadecimal number
#	In:	D3 = Preload value (byte)
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex8_preloaded:
	eor.b	%d1, %d1						/* Clear digit count */
	mov.b	#2, %d2							/* Set max digits */
input_hex8_preloaded_loop:
	rol.b	#4, %d3							/* Rotate to digit */
	mov.b	%d3, %d0						/* Copy digit */
	andi.b	#0xf, %d0						/* Get nibble */
	mov.b	%d0, -(%sp)						/* Push value onto stack */
	jsr	print_hex_digit
	addi.b	#1, %d1							/* Increment digit count */
	cmp.b	%d2, %d1
	bne.s	input_hex8_preloaded_loop				/* Loop through digits */
	bra.s	input_hex_get_char
# input_hex8
#################################
#  Routine to enter up to 2 digit hexadecimal number
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex8:
	eor.b	%d1, %d1						/* Clear digit count */
	mov.b	#2, %d2							/* Set max digits */
	bra.s	input_hex_get_char
# input_hex16_preloaded
#################################
#  Routine to enter up to 4 digit hexadecimal number
#	In:	D3 = Preload value (word)
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex16_preloaded:
	eor.b	%d1, %d1						/* Clear digit count */
	mov.b	#4, %d2							/* Set max digits */
input_hex16_preloaded_loop:
	rol.w	#4, %d3							/* Rotate to digit */
	mov.b	%d3, %d0						/* Copy digit */
	andi.b	#0xf, %d0						/* Get nibble */
	mov.b	%d0, -(%sp)						/* Push value onto stack */
	jsr	print_hex_digit
	addi.b	#1, %d1							/* Increment digit count */
	cmp.b	%d2, %d1
	bne.s	input_hex16_preloaded_loop				/* Loop through digits */
	bra.s	input_hex_get_char
# input_hex16
#################################
#  Routine to enter up to 4 digit hexadecimal number
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex16:
	eor.b	%d1, %d1						/* Clear digit count */
	mov.b	#4, %d2							/* Set max digits */
	bra.s	input_hex_get_char
# input_hex32_preloaded
#################################
#  Routine to enter up to 8 digit hexadecimal number
#	In:	D3 = Preload value (long)
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex32_preloaded:
	eor.b	%d1, %d1						/* Clear digit count */
	mov.b	#8, %d2							/* Set max digits */
input_hex32_preloaded_loop:
	rol.l	#4, %d3							/* Rotate to digit */
	mov.b	%d3, %d0						/* Copy digit */
	andi.b	#0xf, %d0						/* Get nibble */
	mov.b	%d0, -(%sp)						/* Push value onto stack */
	jsr	print_hex_digit
	addi.b	#1, %d1							/* Increment digit count */
	cmp.b	%d2, %d1
	bne.s	input_hex32_preloaded_loop				/* Loop through digits */
	bra.s	input_hex_get_char
# input_hex32
#################################
#  Routine to enter up to 8 digit hexadecimal number
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex32:
	eor.b	%d1, %d1						/* Clear digit count */
	mov.b	#8, %d2							/* Set max digits */
# input_hex_get_char
#################################
#  Base routine to enter hex ASCII digit(s), and convert that to the equivalent hex value.
#	In:	D1 = Current digit count
#		D2 = Maximum number of digits
#	Out:	D0 = Hex value
#		Carry flag set if value valid
input_hex_get_char:
	jsr	input_character_filter					/* Get character */
	jsr	char_2_upper
input_hex_process_char:
	mov.b	%d0, %d3						/* Copy character */

	cmp.b	#ascii_escape, %d0					/* Check whether character is escape key */
	beq.s	input_hex_abort
	cmp.b	#ascii_backspace, %d0					/* Check whether character is backspace key */
	beq.s	input_hex_delete_digit
	cmp.b	#ascii_delete, %d0					/* Check whether character is delete key */
	beq.s	input_hex_delete_digit
	cmp.b	#ascii_carriage_return, %d0				/* Check whether character is CR key */
	beq.s	input_hex_complete
	cmp.b	#ascii_linefeed, %d0					/* Check whether character is CR key */
	beq.s	input_hex_complete

	cmp.b	%d2, %d1						/* Check that number of digits is less than (n) */
	beq.s	input_hex_get_char					/* Already have (n) digits, so just loop */
	jsr	char_2_hex						/* Convert ASCII to hex */
	bcc.s	input_hex_get_char					/* Character not valid hex digit so loop */
	mov.b	%d0, -(%sp)						/* Push hex value on to stack */
	addi.b	#1, %d1							/* Increment digit count */
	mov.b	%d3, %d0						/* Reload character */
	jsr	console_out						/* Output character */
	bra.s	input_hex_get_char					/* Loop */
input_hex_delete_digit:
	and.b	%d1, %d1						/* Check if there are digits to delete */
	beq.s	input_hex_get_char					/* No existing digits, so just wait for next character */
	jsr	console_out						/* Back 1 character */
	mov.b	#' ', %d0						/* Overwrite character */
	jsr	console_out
	mov.b	#ascii_backspace, %d0					/* Back 1 character again */
	jsr	console_out
	mov.b	(%sp)+, %d0						/* Pop digit from stack */
	subi.b	#1, %d1							/* Decrement digit count */
	bra.s	input_hex_get_char					/* Loop */
input_hex_abort:
	and.b	%d1, %d1						/* Check if there's anything to remove from the stack */
	beq.s	input_hex_abort_end					/* Nothing to pop, so finish */
input_hex_abort_loop:
	mov.b	(%sp)+, %d0						/* Pop digit */
	subi.b	#1, %d1							/* Decrement digit count */
	bne.s	input_hex_abort_loop					/* Keep looping until all digits removed */
input_hex_abort_end:
	eor.l	%d0, %d0						/* Zero return register */
	and.b	#0xfe, %ccr						/* Invalid value */
	rts
input_hex_complete:
	eor.l	%d0, %d0						/* Zero return register */
	mov.b	%d1, %d2						/* Copy count, and check if there's anything to remove from the stack */
	beq.s	input_hex_complete_end					/* Nothing to pop, so finish */
input_hex_complete_stack_loop:
	mov.b	(%sp)+, %d3						/* Pop digit */
	or.b	%d3, %d0						/* Merge numbers */
	ror.l	#4, %d0							/* Make some room for a nibble */
	subi.b	#1, %d1							/* Decrement digit count */
	bne.s	input_hex_complete_stack_loop				/* Loop for remaining digits */
input_hex_complete_shift_loop:
	rol.l	#4, %d0							/* Move digits back in to poistion */
	subi.b	#1, %d2
	bne.s	input_hex_complete_shift_loop
input_hex_complete_end:
	ori.b	#0x01, %ccr						/* Valid value */
	rts

# input_addrs_start_end
#################################
#  Routine to get a start and end address
#	Out:	A4 = Start Address
#		A5 = End Address
input_addrs_start_end:
	jsr	print_newlinex2
	lea	str_start_addr, %a0
	jsr	print_cstr
	jsr	input_hex32						/* Get start address */
	bcs.s	input_addrs_start_end_next_addr				/* If it's valid, get next address */
	mov.l	(%sp)+, %d0						/* Dump return address off stack */
	bra.w	print_abort						/* So when this returns, it returns to the menu */
input_addrs_start_end_next_addr:
	mov.l	%d0, %a4						/* Store start address */
	jsr	print_newline
	lea	str_end_addr, %a0
	jsr	print_cstr
	jsr	input_hex32
	bcs.s	input_addrs_start_end_check
	mov.l	(%sp)+, %d0						/* Dump return address off stack */
	bra.w	print_abort						/* So when this returns, it returns to the menu */
input_addrs_start_end_check:
	mov.l	%d0, %a5
	cmp.l	%a5, %a4						/* Check end address is greater (or equal) than start address */
	bgt.s	input_addrs_start_end_invalid
	bra.w	print_newline
input_addrs_start_end_invalid:
	mov.l	(%sp)+, %d0						/* Dump return address off stack */
	jsr	print_newline
	lea	str_invalid, %a0
	bra.w	print_cstr

# input_str
#################################
#  Input a string of upto buffer size - 1. Null terminates string.
#	In:	A4 = Pointer to string buffer
#		D4 = Size of input buffer (size: word)
#	Out:	D1 = Character count (size: long)
#		Carry flag set if value valid
input_str:
	sub.w	#1, %d4							/* Max number of characters is buffer size - 1, to accept null character at end */
	eor.l	%d1, %d1						/* Clear byte count (clear long for other operations) */
	mov.l	%a4, %a0						/* Make working copy of buffer pointer */
input_str_get_char:
	jsr	input_character_filter					/* Get character */

	cmp.b	#ascii_escape, %d0					/* Check whether character is escape key */
	beq.s	input_str_abort
	cmp.b	#ascii_backspace, %d0					/* Check whether character is backspace key */
	beq.s	input_str_delete_char
	cmp.b	#ascii_delete, %d0					/* Check whether character is delete key */
	beq.s	input_str_delete_char
	cmp.b	#ascii_carriage_return, %d0				/* Check whether character is CR key */
	beq.s	input_str_complete
	cmp.b	#ascii_linefeed, %d0					/* Check whether character is CR key */
	beq.s	input_str_complete

	cmp.w	%d4, %d1						/* Compare character count to max. characters */
	beq.s	input_str_get_char					/* If equal, do nothing */
	mov.b	%d0, (%a0)+						/* Copy character to string buffer */
	jsr	monitor_out						/* Print character */
	addq.w	#1, %d1							/* Increment byte count */
	bra.s	input_str_get_char
input_str_delete_char:
	and.w	%d1, %d1						/* Check if there are characters to delete */
	beq.s	input_str_get_char					/* No existing characters, so just wait for next character */
	jsr	monitor_out						/* Back 1 character */
	mov.b	#' ', %d0
	jsr	monitor_out						/* Overwrite character */
	mov.b	#ascii_backspace, %d0
	jsr	monitor_out						/* Back 1 character again */
	suba.l	#1, %a0							/* Decrement string buffer pointer */
	subq.w	#1, %d1							/* Decrement byte count */
	bra.s	input_str_get_char
input_str_complete:
	eor.b	%d0, %d0						/* Clear register */
	mov.b	%d0, (%a0)						/* Write null character to buffer */
	ori.b	#0x01, %ccr						/* Valid value */
	rts
input_str_abort:
	and.b	#0xfe, %ccr						/* Invalid value */
	rts

# Memory routines
###########################################################################
# memory_copy
#################################
#  Copies a region of memory to another region
#	In:	A0 = Source Start Address
#		A1 = Source End Address
#		A2 = Destination Address
memory_copy:
	cmp.l	%a1, %a0
	bgt.s	memory_copy_finish					/* Check if source is greater than destination */
	mov.l	%a1, %d0
	sub.l	%a0, %d0						/* Calculate number of bytes to copy */
	add.l	#1, %d0							/* If src addr == dst addr, still copying 1 byte */
memory_copy_long_loop:
	cmp.l	#4, %d0							/* Check if there are more than 4 bytes to copy */
	blt.s	memory_copy_byte_loop
	mov.l	(%a0)+, (%a2)+						/* Copy 4 bytes */
	sub.l	#4, %d0							/* Subtract 4 from byte count */
	bra.s	memory_copy_long_loop					/* Loop */
memory_copy_byte_loop:
	tst.b	%d0							/* Check if there are any more bytes to copy */
	beq.s	memory_copy_finish
	mov.b	(%a0)+, (%a2)+						/* Copy byte */
	sub.b	#1, %d0							/* Subtract from byte count */
	bra.s	memory_copy_byte_loop					/* Loop */
memory_copy_finish:
	rts

*; # memory_copy_verify
*; #################################
*;  Verifies a region of memory against another region
*;	In:	BC = Byte count
*;		DE = Source address
*;		HL = Target address
*;	Out:	Carry flag set if okay, unset if not
*memory_copy_verify:
*	ld	a, (de)					; Get source byte
*	cp	(hl)					; Compare against target byte
*	jr	nz, memory_copy_verify_failed
*	inc	de					; Increment pointers
*	inc	hl
*	dec	bc					; Decrement byte count
*	ld	a, c
*	or	b					; See if we've reached zero
*	jr	nz, memory_copy_verify			; If there are remaining bytes to check
*	scf
*	ret
*memory_copy_verify_failed:
*	scf						; Clear Carry flag
*	ccf
*	ret

# Math routines
###########################################################################
*# math_divide_16b
*#################################
*#  Divides 16 bit number by another 16 bit number
*#  From: http://map.grauw.nl/sources/external/z80bits.html
*#	In:	BC = Dividend
*#		DE = Divisor
*#		HL = 0
*#	Out:	BC = Quotient
*#		HL = Remainder
*math_divide_16b:
*	ld	hl, 0x0000
*	ld	a, 0x10
*math_divide_16b_loop:
*	db	 0xcb, 0x31				; Undocumented instruction: SLL  C
*	;sll	c					; carry <- C <- 1
*	rl	b					; carry <- B <- carry
*	adc	hl, hl					; HL = HL + HL + carry
*	sbc	hl, de					; HL = HL - (DE + carry)
*	jr	nc, math_divide_16b_check
*	add	hl, de					; HL = HL + DE
*	dec	c					; C = C - 1
*math_divide_16b_check:
*	dec	a
*	jr	nz, math_divide_16b_loop
*	ret

*; # math_bcd_2_hex
*; #################################
*;  Converts an 8 bit bcd encoded value to hex
*;	In:	A = BCD value
*;	Out:	A = Hex value
*math_bcd_2_hex:
*	ld	b, a					; Save value
*	srl	b					; Shift upper nibble to lower
*	srl	b
*	srl	b
*	srl	b
*	and	0x0f					; Extract lower nibble
*	ld	c, a					; Save for later
*	xor	a					; Clear A
*	cp	b					; Test 10s digit
*	jr	z, math_bcd_2_hex_combine		; No 10s, so finish
*math_bcd_2_hex_loop:
*	add	0x0a					; Add 10
*	djnz	math_bcd_2_hex_loop
*math_bcd_2_hex_combine:
*	add	c					; Combine values
*	ret

# Library routines implemented with TRAPs
#  D7 selects subroutine
###########################################################################
# TRAP0 (console routines)
#################################
trap_0:
# trap_0_0 (Console out)
#################################
#  Writes a byte of data to console port
#	In:	D0 = Byte to write out
trap_0_0:
	cmp.b	#0, %d7
	bne.s	trap_0_1
	jsr	console_out
	rte
# trap_0_1 (Console in, blocking)
#################################
#  Reads a byte of data from console port
#	Out:	D0 = Byte read
trap_0_1:
	cmp.b	#1, %d7
	bne.s	trap_0_2
	jsr	console_in
	rte
# trap_0_2 (Console in, non-blocking)
#################################
#  Reads a byte of data from console port
#	Out:	D0 = Byte read
trap_0_2:
	cmp.b	#2, %d7
	bne.s	trap_0_3
	jsr	console_in_nonblock
	rte
# trap_0_3 (Console in check)
#################################
#  Check whether there is a byte of data
#  available from console port
#	Out:	Z cleared if byte available
trap_0_3:
	cmp.b	#3, %d7
	bne.s	trap_0_other
	jsr	console_in_check
trap_0_other:
	rte

# Main routines
###########################################################################
# module_list_commands
#################################
#  Print a list of additional commands
command_module_list_commands:
	lea	str_tag_listm, %a0
	jsr	print_cstr						/* Print message */
module_list_commands:
	lea	str_prompt9, %a0
	jsr	print_cstr
	mov.b	#' ', %d0
	mov.w	#21, %d1
	jsr	print_char_n
	#lea	str_prompt9b, %a0
	jsr	print_cstr

	lea	module_search_mem_start, %a4				/* Set search start */
	lea	module_search_mem_end, %a5				/* Set search end */
module_list_commands_next:
	lea	module_list_commands_reenter, %a6			/* Set return address */
	bra.w	module_find						/* Find next module */
module_list_commands_reenter:
	beq.w	module_list_commands_exit				/* If not, return */

	jsr	print_spacex2
	mov.l	%a4, %a0						/* Reset offset to command name */
	adda.w	#0x20, %a0
	jsr	print_str						/* Print command name */
	mov.l	%a4, %a0						/* Reset offset to command name */
	adda.w	#0x20, %a0
	jsr	string_length						/* Get command name length */
	mov.w	#31, %d1						/* Need word here */
	sub.b	%d0, %d1						/* Calculate padding */
	mov.b	#' ', %d0
	jsr	print_char_n						/* Print padding */
	mov.l	%a4, %d3						/* Reset offset to module start */
	jsr	print_hex32						/* Print module address */
	mov.b	#' ', %d0
	mov.w	#4, %d1
	jsr	print_char_n						/* Print padding */
	mov.l	%a4, %a0						/* Reset offset to command type */
	adda.w	#0x04, %a0
	mov.b	(%a0), %d0						/* Get command type */

	lea	str_type5, %a0						/* Type unknown */
module_list_commands_type_35:
	cmp.b	#35, %d0						/* Program */
	bne.s	module_list_commands_type_249
	lea	str_type2, %a0
	bra.s	module_list_commands_type_print
module_list_commands_type_249:
	cmp.b	#249, %d0						/* Init */
	bne.s	module_list_commands_type_253
	lea	str_type3, %a0
	bra.s	module_list_commands_type_print
module_list_commands_type_253:
	cmp.b	#253, %d0						/* Startup command */
	bne.s	module_list_commands_type_254
	lea	str_type4, %a0
	bra.s	module_list_commands_type_print
module_list_commands_type_254:
	cmp.b	#254, %d0						/* External command */
	bne.s	module_list_commands_type_print
	lea	str_type1, %a0
module_list_commands_type_print:
	jsr	print_cstr						/* Print type */
	jsr	print_newline

	adda.l	#0x100, %a4						/* Increment address pointer */
	cmp.l	%a5, %a4						/* Check if we've reached the end */
	blt.w	module_list_commands_next				/* Loop */
module_list_commands_exit:
	jmp	print_newline						/* Finish */

# command_help_line_print
#################################
#  Prints a line of help text
#	In:	D1 = Command key
#		A0 = Command help string
command_help_line_print:
	jsr	print_spacex2
	mov.b	%d1, %d0
	jsr	console_out
	jsr	print_dash_spaces
	jsr	print_cstr
	jmp	print_newline

# command_help
#################################
#  Prints help text
command_help:
	lea	str_tag_help2, %a0
	jsr	print_cstr						/* Print message */
command_help_internal_commands:
	lea	str_help1, %a0
	jsr	print_cstr

	mov.b	#command_key_help, %d1
	lea	str_tag_help1, %a0
	jsr	command_help_line_print
	mov.b	#command_key_listm, %d1
	lea	str_tag_listm, %a0
	jsr	command_help_line_print
#	mov.b	#command_key_run, %d1
#	lea	str_tag_run, %a0
#	jsr	command_help_line_print
	mov.b	#command_key_download, %d1
	lea	str_tag_dnld, %a0
	jsr	command_help_line_print
	mov.b	#command_key_upload, %d1
	lea	str_tag_upld, %a0
	jsr	command_help_line_print
	mov.b	#command_key_new_locat, %d1
	lea	str_tag_nloc, %a0
	jsr	command_help_line_print
	mov.b	#command_key_jump, %d1
	lea	str_tag_jump, %a0
	jsr	command_help_line_print
	mov.b	#command_key_hexdump, %d1
	lea	str_tag_hexdump, %a0
	jsr	command_help_line_print
	mov.b	#command_key_regdump, %d1
	lea	str_tag_regdump, %a0
	jsr	command_help_line_print
	mov.b	#command_key_edit, %d1
	lea	str_tag_edit, %a0
	jsr	command_help_line_print
	mov.b	#command_key_clrmem, %d1
	lea	str_tag_clrmem, %a0
	jsr	command_help_line_print
	mov.b	#command_key_stack, %d1
	lea	str_tag_stack, %a0
	jsr	command_help_line_print

command_help_external_commands:
	lea	str_help2, %a0
	jsr	print_cstr
	lea	module_search_mem_start, %a4				/* Set search start */
command_help_external_commands_loop:
	mov.b	#0xfe, %d1						/* Search for external command */
	jsr	module_search
	beq.s	command_help_end					/* No command found so finish */
	jsr	print_spacex2
	mov.l	%a4, %a0						/* Offset to module command character */
	adda.w	#0x5, %a0
	mov.b	(%a0), %d0						/* Get module command character */
	jsr	console_out						/* Print character */
	jsr	print_dash_spaces
	mov.l	%a4, %a0						/* Offset to module command name */
	adda.w	#0x20, %a0
	jsr	print_str						/* Print module name */
	jsr	print_newline
	adda.w	#0x100, %a4						/* Increment module search start address */
	bra.s	command_help_external_commands_loop
command_help_end:
	jmp	print_newline						/* Print newline and return */

# command_location_new
#################################
#  Sets the monitor pointer to where default operations are performed
command_location_new:
	lea	str_tag_nloc, %a0
	jsr	print_cstr						/* Print message */

	lea	str_prompt6, %a0					/* Print location prompt */
	jsr	print_cstr
	mov.l	MONITOR_ADDR_CURRENT, %d3				/* Preload current address */
	jsr	input_hex32_preloaded					/* Get value */
	bcc.w	print_abort						/* If escaped, print abort message */
	mov.l	%d0, MONITOR_ADDR_CURRENT				/* Save value */
	jmp	print_newline

# command_stack_change
#################################
#  Sets the monitor pointer to where default operations are performed
command_stack_change:
	lea	str_tag_stack, %a0
	jsr	print_cstr						/* Print message */

	lea	str_prompt14, %a0					/* Print location prompt */
	jsr	print_cstr
	jsr	input_hex32						/* Get value */
	bcc.w	print_abort						/* If escaped, print abort message */
	mov.l	%d0, %sp						/* Set stack pointer */
	jmp	startup_final

# command_jump
#################################
#  Request an address, and jump to the code at that location
command_jump:
	lea	str_tag_jump, %a0
	jsr	print_cstr						/* Print message */

	lea	str_prompt8, %a0
	jsr	print_cstr
	lea	str_prompt4, %a0
	jsr	print_cstr
	mov.l	MONITOR_ADDR_CURRENT, %d3
	jsr	input_hex32_preloaded
	bcc.w	print_abort						/* If escaped, print abort message */
	mov.l	%d0, %a6						/* Copy address */
	lea	str_runs, %a0
	jsr	print_cstr
	mov.l	%a6, %d3
	jsr	print_hex32
	jsr	print_newline

	lea	startup_warm, %a0					/* Push reset address on stack */
	mov.l	%a0, -(%sp)						/* In case there's a RTS at the end of the code */
command_jump_brkpnt:
	jmp	(%a6)							/* Execute code */

# command_hexdump
#################################
#  Dump memory at the default location
command_hexdump:
	lea	str_tag_hexdump, %a0
	jsr	print_cstr						/* Print message */
	jsr	print_newline

	mov.l	MONITOR_ADDR_CURRENT, %a4

command_hexdump_page_print:
	mov.w	#0x07, %d5						/* Number of bytes to print */
command_hexdump_line_print:
	mov.w	#0x07, %d4						/* Number of bytes per line */
	jsr	print_spacex2
	mov.l	%a4, %d3
	jsr	print_hex32
	jsr	print_colon_space
command_hexdump_line_print_loop:
	mov.b	(%a4)+, %d1						/* Get byte of memory */
	jsr	print_hex8
	mov.b	(%a4)+, %d1						/* Get byte of memory */
	jsr	print_hex8
	jsr	print_spacex2
	dbf	%d4, command_hexdump_line_print_loop
	jsr	print_newline
	dbf	%d5, command_hexdump_line_print

	lea	str_prompt15, %a0
	jsr	print_cstr
	jsr	input_character_filter
	cmp.b	#ascii_escape, %d0					/* Check if quit */
	beq.s	command_hexdump_end
	bra.s	command_hexdump_page_print				/* Loop */
command_hexdump_end:
	rts

# command_edit
#################################
#  Basic memory editor
command_edit:
	lea	str_tag_edit, %a0
	jsr	print_cstr						/* Print message */

	lea	str_edit1, %a0
	jsr	print_cstr
	mov.l	MONITOR_ADDR_CURRENT, %a4				/* Get default address */
command_edit_loop:
	mov.l	%a4, %d3
	jsr	print_hex32						/* Print address */
	jsr	print_colon_space
	mov.b	(%a4), %d3						/* Load memory contents */
	jsr	input_hex8_preloaded					/* Edit loaded value */
	bcc.w	print_abort						/* Check if Escape was pressed */
command_edit_save:
	mov.b	%d0, (%a4)+						/* Save editted value, increment pointer */
	mov.l	%a4, MONITOR_ADDR_CURRENT				/* Save memory pointer as default */
	jsr	print_newline
	bra.s	command_edit_loop					/* Loop */

# command_clear_mem
#################################
#  Clears a region of memory
command_clear_mem:
	lea	str_tag_clrmem, %a0
	jsr	print_cstr						/* Print message */

	jsr	input_addrs_start_end
	lea	str_sure, %a0
	jsr	print_cstr
	jsr	input_character_filter					/* Get response */
	jsr	char_2_upper
	cmp.b	#'Y', %d0						/* Compare key to 'Y' */
	beq.s	command_clear_mem_do
	mov.b	#'N', %d0
	jsr	console_out
	jmp	print_newlinex2
command_clear_mem_do:
	mov.b	#'Y', %d0
	jsr	console_out
	jsr	print_newline
	eor.b	%d0, %d0						/* Clear D0 */
command_clear_mem_loop:
	mov.b	%d0, (%a4)+						/* Clear memory, increment pointer */
	cmp.l	%a5, %a4						/* Compare start/end addresses */
	ble.s	command_clear_mem_loop
	lea	str_clrcomp, %a0
	jsr	print_cstr
	rts

*;; # command_run
*;; #################################
*;;  Lists module with id code (35), and provides the ability to run them.
*;command_run:
*;	ld	hl, str_tag_run
*;	call	print_cstr				; Print message
*;
*;	call	print_newlinex2
*;	ld	b, 0					; Module count
*;	ld	hl, mem_srch_start			; Set search start
*;command_run_module_list_loop:
*;	ld	a, 0x23					; Search for program modules
*;	push	bc					; Save BC
*;	call	module_search
*;	pop	bc					; Restore BC
*;	jr	nc, command_run_any_programs		; No command found so finish
*;	inc	b					; Found a program
*;	call	print_spacex2
*;	ld	a, 'A'-1				; -1 because it's added to B which is effectively +1
*;	add	a, b					; Create character to press to run program
*;	call	monlib_console_out			; Print character
*;	call	print_dash_spaces
*;	ld	l, 0x20					; Offset to module name
*;	call	print_str				; Print module name
*;	call	print_newline
*;	inc	h					; Increment module search start address
*;	jr	command_run_module_list_loop		; Loop around again
*;command_run_any_programs:
*;	ld	a, b					; Check if there are any programs
*;	and	a
*;	jr	nz, command_run_select_module
*;command_run_no_programs:
*;	ld	hl, str_prompt5
*;	jp	print_cstr
*;command_run_select_module:
*;	push	bc					; Save B again
*;	ld	hl, str_prompt3				; Print select
*;	call	print_cstr
*;	ld	a, 'A'					; First character
*;	call	monlib_console_out
*;	call	print_dash
*;	ld	a, 'A'-1				; Last character to select program
*;	pop	bc					; Restore B
*;	add	a, b
*;	call	monlib_console_out
*;	push	bc
*;	ld	hl, str_prompt4
*;	ld	a, ')'
*;	call	monlib_console_out
*;	call	print_cstr
*;	call	input_character_filter			; Get character to select program
*;	cp	character_code_escape
*;	jr	nz, command_run_validate_selection	; Check that it wasn't escape
*;	pop	bc					; If it was pop stored BC
*;	jp	print_abort
*;command_run_validate_selection:
*;	pop	bc
*;	call	char_2_upper
*;	cp	'A'					; Check that it's A or higher
*;	jr	c, command_run_invalid
*;	ld	c, a					; Save character
*;	ld	a, 'A'-1
*;	add	b					; Calculate last letter
*;	cp	c					; Check that it's less than or equal to second letter
*;	jr	nc, command_run_get_program
*;	jr	z, command_run_get_program
*;command_run_invalid:
*;	ld	hl, str_invalid
*;	jp	print_cstr
*;command_run_get_program:
*;	ld	a, c
*;	call	monlib_console_out			; Print selected character
*;	ld	a, c					; Get character code
*;	sub	'A'-1					; Calculate the index
*;	ld	b, a					; Save index
*;	call	print_newline
*;	ld	hl, mem_srch_start			; Set search start
*;command_run_get_program_loop:
*;	push	bc					; Save BC
*;	ld	a, 0x23					; Search for program modules
*;	call	module_search
*;	jr	nc, command_run_invalid			; This should never run
*;	pop	bc
*;	inc	h					; Increment module search pointer
*;	djnz	command_run_get_program_loop
*;	dec	h
*;	ld	l, 0x40					; Set offset to program code
*;command_run_brkpnt:
*;	jp	(hl)					; Execute program code

# command_upload_srec
#################################
#  Uploads a selected section of memory in SREC format
command_upload_srec:
	lea	str_tag_upld, %a0
	jsr	print_cstr						/* Print message */

	jsr	input_addrs_start_end
	lea	str_upld1, %a0
	jsr	print_cstr
	mov.l	%a4, %d3
	jsr	print_hex32						/* Print start address */
	lea	str_upld2, %a0
	jsr	print_str
	mov.l	%a5, %d3
	jsr	print_hex32						/* Print end address */
	jsr	print_newline
	lea	str_prompt7, %a0
	jsr	print_cstr
	jsr	console_in						/* Get character */
	cmp.b	#ascii_escape, %d0					/* Check if escape */
	beq.w	print_abort
command_upload_srec_send:
	jsr	print_newline
	jsr	command_upload_srec_S0					/* Print SREC header */
	mov.l	%a4, %a0						/* Working copy of start address */
command_upload_srec_send_calc_remaining:
	cmp.l	%a5, %a0						/* Check start address against end address */
	bgt.s	command_upload_srec_finish
	mov.l	%a0, %a1						/* Calculate default end address */
	adda	#0x1f, %a1
	mov.l	%a5, %d0						/* Calculate remaining bytes */
	sub.l	%a0, %d0
	cmp.l	#0x1f, %d0						/* Check remaining bytes */
	bgt.s	command_upload_srec_send_record
	mov.l	%a5, %a1						/* Set end address (from end address) */
command_upload_srec_send_record:
	jsr	command_upload_srec_S3					/* Send data */
	bra.s	command_upload_srec_send_calc_remaining
command_upload_srec_finish:
	jsr	command_upload_srec_S7					/* Send start address */
	rts

# Output SREC header
#################################
#  Outputs a SREC header
command_upload_srec_S0:
	mov.b	#'S', %d0						/* Header character */
	jsr	console_out
	mov.b	#'0', %d0						/* Header type */
	jsr	console_out
	eor.w	%d4, %d4						/* Clear register to calculate checksum */
	lea	str_upld_srec_hdr, %a0					/* Calculate header length */
	jsr	string_length
	mov.b	%d0, %d1
	addq.b	#4, %d1
	mov.b	%d1, %d4						/* First part of checksum */
	jsr	print_hex8						/* Print length in bytes */
	eor.w	%d1, %d1						/* Clear register to load byte into */
	jsr	print_hex8						/* Print address */
	jsr	print_hex8
	lea	str_upld_srec_hdr, %a0
command_upload_srec_S0_loop:
	mov.b	(%a0)+, %d1						/* Get header byte */
	beq.s	command_upload_srec_S0_finish
	add.w	%d1, %d4						/* Add byte to the checksum value */
	jsr	print_hex8
	bra.s	command_upload_srec_S0_loop				/* Loop */
command_upload_srec_S0_finish:
	eor.b	%d1, %d1
	jsr	print_hex8						/* Null character for string */
	mov.b	#0xff, %d1						/* Calculate checksum */
	sub.b	%d4, %d1
	jsr	print_hex8
	jsr	print_newline
	rts

# Output SREC data
#################################
#  Output an address range in SREC format
#	Out:	A0 = Start address
#		A1 = End address
command_upload_srec_S3:
	mov.b	#'S', %d0						/* Header character */
	jsr	console_out
	mov.b	#'3', %d0						/* Header type */
	jsr	console_out
	eor.w	%d4, %d4						/* Clear register to calculate checksum */
	mov.l	%a1, %d3						/* Copy end address */
	sub.l	%a0, %d3						/* End address - Start address (number of bytes to copy - 1) */
	mov.b	%d3, %d1
	add.b	#6, %d1							/* Byte count = (end - start) + 1(byte of memory) + 4(address) + 1(checksum) */
	mov.b	%d1, %d4						/* First part of checksum */
	jsr	print_hex8						/* Print length in bytes */
	mov.l	%a0, %d2						/* Copy start address to shift */
	mov.w	#3, %d5							/* Number of shifts to output address */
	eor.w	%d1, %d1						/* Clear D1 for add.w */
command_upload_srec_S3_addr_loop:
	rol.l	#8, %d2							/* Shift byte of start address */
	mov.b	%d2, %d1						/* Copy LSB */
	add.w	%d1, %d4						/* Add to checksum */
	jsr	print_hex8
	dbf	%d5, command_upload_srec_S3_addr_loop
command_upload_srec_S3_data_loop:
	mov.b	(%a0)+, %d1						/* Get byte of memory */
	add.w	%d1, %d4						/* Add byte to the checksum value */
	jsr	print_hex8
	dbf	%d3, command_upload_srec_S3_data_loop			/* Loop over remaining data */
command_upload_srec_S3_finish:
	mov.b	#0xff, %d1						/* Calculate checksum */
	sub.b	%d4, %d1
	jsr	print_hex8
	jsr	print_newline
	rts

# Output SREC start address
#################################
#  Outputs a SREC start address record
command_upload_srec_S7:
	mov.b	#'S', %d0						/* Header character */
	jsr	console_out
	mov.b	#'7', %d0						/* Header type */
	jsr	console_out
	eor.w	%d4, %d4						/* Clear register to calculate checksum */
	mov.w	#5, %d1
	add.w	%d1, %d4						/* Add byte count */
	jsr	print_hex8
	eor.l	%d3, %d3						/* Clear register */
	jsr	print_hex32
command_upload_srec_S7_finish:
	mov.b	#0xff, %d1						/* Calculate checksum */
	sub.b	%d4, %d1
	jsr	print_hex8
	jsr	print_newline
	rts

# command_download_srec
#################################
#  Downloads a SREC format hex file
#	Storage: MONITOR_ADDR_TEMP[x]
#		0 = lines received		4 = wrong byte size
#		1 = bytes received		5 = incorrect checksums
#		2 = incorrect line start	6 = bytes unable to write
#		3 = unexpected non-hex digits	7 = bytes written
#	Registers:
#		A4 = String buffer pointer
#		A5 = SREC data address
#		D4 = Line character count
#		D5 = SREC type
#		D6 = SREC byte count
#		D7 = SREC checksum
command_download_srec:
	lea	str_tag_dnld, %a0
	jsr	print_cstr						/* Print message */
	lea	str_dnld1, %a0
	jsr	print_cstr

	lea	MONITOR_ADDR_TEMP0, %a0
	mov.l	#0, (%a0)+						/* Initialise temporary storage */
	mov.l	#0, (%a0)+
	mov.l	#0, (%a0)+
	mov.l	#0, (%a0)+
	mov.l	#0, (%a0)+
	mov.l	#0, (%a0)+
	mov.l	#0, (%a0)+
	mov.l	#0, (%a0)+
command_download_srec_read_line:
	bclr.b	#MONITOR_CONFIG_PRINT, (MONITOR_ADDR_ATTRIBUTES1)	/* Disable printing */
	lea	MONITOR_ADDR_STR_BUFFER, %a4
	mov.w	#MONITOR_ADDR_STR_BUFFER_SIZE, %d4
	jsr	input_str						/* Get a SREC record */
	bset.b	#MONITOR_CONFIG_PRINT, (MONITOR_ADDR_ATTRIBUTES1)	/* Re-enable printing */
	bcs.s	command_download_srec_check_line
	lea	str_dnld2, %a0
	jsr	print_cstr						/* Print string */
	rts								/* Escape received, so exit */
command_download_srec_check_line:
	mov.l	%d1, %d4						/* Save string length */

	mov.l	(MONITOR_ADDR_TEMP0), %d0				/* Check line count */
	andi.b	#0x7f, %d0						/* Should produce a newline every 80 transfers */
	cmp.b	#0x0, %d0
	bne.s	command_download_srec_check_line_inc_stats
	jsr	print_newline

command_download_srec_check_line_inc_stats:
	lea	MONITOR_ADDR_TEMP0, %a0
	add.l	#1, (%a0)+						/* Increment line count */
	add.l	%d4, (%a0)+						/* Increment byte count */
	cmp.b	#'S', (%a4)+						/* Check for line start character */
	beq.s	command_download_srec_check_line_checksum
	add.l	#1, (%a0)+						/* Increment incorrect line start count */
	bra.s	command_download_srec_read_line				/* Incorrect line start, so loop */
command_download_srec_check_line_checksum:
	mov.l	%a4, %a0						/* Copy buffer pointer */
	eor.l	%d1, %d1						/* Clear register */
	jsr	string_2_hex						/* Get SREC type */
	bcc.s	command_download_srec_check_line_invalid_char		/* Not a hex character */
	mov.l	%d1, %d5						/* Store SREC type */
	jsr	string_2_hex8						/* Get byte count */
	bcc.s	command_download_srec_check_line_invalid_char		/* Not a hex character */
	mov.l	%d1, %d7						/* Start calculating checksum */
	mov.l	%d1, %d6						/* Store SREC byte count */
	subi.l	#4, %d4							/* Delete header and size characters */
	divu.w	#2, %d4							/* Convert received character count to byte count */
	cmp.l	%d4, %d6						/* Compare the given byte length with actual byte length */
	bne.s	command_download_srec_check_line_invalid_length		/* Byte size doesn't match received bytes */
	subi.l	#2, %d4							/* Decrement count to use with dbf */
command_download_srec_check_line_checksum_calc:
	jsr	string_2_hex8						/* Get next byte */
	add.l	%d1, %d7						/* Add to checksum */
	dbf	%d4, command_download_srec_check_line_checksum_calc	/* Loop over remaining data bytes */
	mov.b	#0xff, %d4
	sub.b	%d7, %d4						/* Calculate checksum */
	jsr	string_2_hex8						/* Get checksum byte */
	cmp.b	%d1, %d4						/* Compare checksum values */
	bne.s	command_download_srec_check_line_invalid_checksum	/* Checksums aren't equal */

	cmp.b	#0, %d5							/* Check for SREC type 0 */
	beq.s	command_download_srec_type0
	cmp.b	#3, %d5							/* Check for SREC type 3 */
	beq.s	command_download_srec_type3
	cmp.b	#5, %d5							/* Check for SREC type 5 */
	beq.w	command_download_srec_type5
	cmp.b	#7, %d5							/* Check for SREC type 7 */
	beq.w	command_download_srec_type7

	lea	MONITOR_ADDR_TEMP6, %a0
	add.l	%d6, (%a0)						/* Increment bytes unable to write count */
	bra.w	command_download_srec_read_line				/* Next line */
command_download_srec_check_line_invalid_char:
	lea	MONITOR_ADDR_TEMP3, %a0
	add.l	#1, (%a0)						/* Increment non-hex digits count */
	bra.w	command_download_srec_read_line				/* Next line */
command_download_srec_check_line_invalid_length:
	lea	MONITOR_ADDR_TEMP4, %a0
	add.l	#1, (%a0)						/* Increment wrong byte size count */
	bra.w	command_download_srec_read_line				/* Next line */
command_download_srec_check_line_invalid_checksum:
	lea	MONITOR_ADDR_TEMP5, %a0
	add.l	#1, (%a0)						/* Increment wrong byte size count */
	bra.w	command_download_srec_read_line				/* Next line */

command_download_srec_type0:
	subi.l	#4, %d6							/* Decrement count to use with dbf */
	lea	str_recving, %a0
	jsr	print_str_simple
	lea	MONITOR_ADDR_STR_BUFFER+8, %a0				/* Print SREC header string */
command_download_srec_type0_loop:
	jsr	string_2_hex8						/* Get character */
	mov.b	%d1, %d0
	jsr	console_out						/* Print character */
	dbf	%d6, command_download_srec_type0_loop			/* Loop over remaining characters */
	jsr	print_newline
	bra.w	command_download_srec_read_line				/* Next line */

command_download_srec_type3:
	subi.l	#5, %d6							/* Decrement for byte count */
	lea	MONITOR_ADDR_TEMP7, %a0
	add.l	%d6, (%a0)						/* Increment bytes written count */
	subi.l	#1, %d6							/* Decrement count to use with dbf */
	lea	MONITOR_ADDR_STR_BUFFER+4, %a0
	jsr	string_2_hex32						/* Get address */
	mov.l	%d1, %a5						/* Save SREC data address */
command_download_srec_type3_loop:
	jsr	string_2_hex8						/* Get byte of data */
	mov.b	%d1, (%a5)+						/* Move data into memory */
	dbf	%d6, command_download_srec_type3_loop			/* Loop over remaining bytes */

	mov.b	#'.', %d0						/* Indicate reception */
	jsr	console_out
	bra.w	command_download_srec_read_line				/* Next line */

command_download_srec_type5:						/* Ignore SREC record count record */
	bra.w	command_download_srec_read_line				/* Next line */

command_download_srec_type7:
	lea	MONITOR_ADDR_STR_BUFFER+4, %a0
	jsr	string_2_hex32						/* Get address */
	mov.l	%d1, (MONITOR_ADDR_CURRENT)				/* Save SREC data address */

	jsr	print_newline
	lea	str_start_addr, %a0
	jsr	print_cstr						/* Print string */
	mov.l	(MONITOR_ADDR_CURRENT), %d0
	jsr	print_hex32						/* Print start address */

	lea	str_dnld3, %a0
	jsr	print_cstr						/* Print string */

# Prints download stats
command_download_srec_summary:
	lea	str_dnld4, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP0), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld5, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP1), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld6a, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP7), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld6b, %a0
	jsr	print_cstr

	cmp.l	#0x0, (MONITOR_ADDR_TEMP2)
	bne.s	command_download_summary_error_print
	cmp.l	#0x0, (MONITOR_ADDR_TEMP3)
	bne.s	command_download_summary_error_print
	cmp.l	#0x0, (MONITOR_ADDR_TEMP4)
	bne.s	command_download_summary_error_print
	cmp.l	#0x0, (MONITOR_ADDR_TEMP5)
	bne.s	command_download_summary_error_print
	cmp.l	#0x0, (MONITOR_ADDR_TEMP6)
	bne.s	command_download_summary_error_print

	lea	str_dnld13, %a0						/* No errors */
	jsr	print_cstr
	bra.w	command_download_summary_finish

command_download_summary_error_print:
	lea	str_dnld7, %a0
	jsr	print_cstr						/* Print string */

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP6), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld8, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP5), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld9, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP2), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld10, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP3), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld11, %a0
	jsr	print_cstr

	jsr	print_space
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	mov.l	(MONITOR_ADDR_TEMP4), %d0
	jsr	string_hex_2_dec
	lea	MONITOR_ADDR_STR_BUFFER, %a0
	jsr	print_str_simple
	lea	str_dnld12, %a0
	jsr	print_cstr
command_download_summary_finish:
	jmp	print_newline

*; A short delay since most terminal emulation programs
*; won't be ready to receive anything immediately after
*; they've transmitted a file... even on a fast Pentium(tm)
*; machine with 16550 uarts!
*download_delay:
*	ld	bc, 0x0000
*download_delay_loop:
*	dec	bc
*	jr	nz, download_delay_loop
*	ret

# menu_main
#################################
#  Implements interactive menu
menu_main:
	# First we print out the prompt, which isn't as simple
	# as it may seem, since external code can add to the
	# prompt, so we've got to find and execute all of 'em.
	lea	str_prompt1, %a0					/* First part of the prompt */
	jsr	print_cstr
	mov.l	MONITOR_ADDR_CURRENT, %d3				/* Get current address */
	jsr	print_hex32
	lea	str_prompt2, %a0					/* Second part of the prompt */
	jsr	print_str

	jsr	input_character_filter					/* Get character input */

menu_main_push_address:
	jsr	char_2_upper						/* Convert to uppercase to simplify matching */
	lea	menu_main, %a0
	mov.l	%a0, -(%sp)						/* Push menu_main address to stack to make returning easier */
	mov.b	%d0, %d4						/* Save command character */

menu_main_external_commands:
	lea	module_search_mem_start, %a4				/* Set search start */
menu_main_external_commands_loop:
	mov.b	#0xfe, %d1						/* Search for external command */
	jsr	module_search
	beq.s	menu_main_builtin_commands				/* No command found so procede with builtin commands */
	mov.l	%a4, %a0						/* Make offset to command character */
	adda.w	#0x5, %a0
	cmp.b	(%a0), %d4						/* Are they the same character? */
	beq.s	menu_main_external_commands_exec			/* Execute external command */
	adda.w	#0x100, %a4						/* Increment module search start address */
	bra.s	menu_main_external_commands_loop			/* Loop */
menu_main_external_commands_exec:
	jsr	print_space
	mov.l	%a4, %a0						/* Make offset to command name */
	adda.w	#0x20, %a0
	jsr	print_str						/* Print module command name */
	jsr	print_newline
	mov.l	%a4, %a0						/* Make offset to module code */
	adda.w	#0x40, %a0
	jmp	(%a0)							/* Execute external command */

menu_main_builtin_commands:
	cmp.b	#command_key_help, %d4					/* Check if help key */
	beq.w	command_help						/* Run command */
	cmp.b	#command_key_listm, %d4					/* Check if list modules key */
	beq.w	command_module_list_commands				/* Run command */
	cmp.b	#command_key_regdump, %d4				/* Check if list modules key */
	beq.w	command_print_registers					/* Run command */
	cmp.b	#command_key_new_locat, %d4				/* Check if new location key */
	beq.w	command_location_new					/* Run command */
	cmp.b	#command_key_stack, %d4					/* Check if set stack key */
	beq.w	command_stack_change					/* Run command */
	cmp.b	#command_key_jump, %d4					/* Check if jump key */
	beq.w	command_jump						/* Run command */
	cmp.b	#command_key_hexdump, %d4				/* Check if hexdump key */
	beq.w	command_hexdump						/* Run command */
	cmp.b	#command_key_edit, %d4					/* Check if edit key */
	beq.w	command_edit						/* Run command */
	cmp.b	#command_key_clrmem, %d4				/* Check if clear memory key */
	beq.w	command_clear_mem					/* Run command */
	cmp.b	#command_key_upload, %d4				/* Check if upload key */
	beq.w	command_upload_srec					/* Run command */
	cmp.b	#command_key_download, %d4				/* Check if download key */
	beq.w	command_download_srec					/* Run command */

menu_main_end:
	jmp	print_newline						/* This will return to menu_main */

# Fixed data structures
###########################################################################

# Dictionary of common words
#################################
#  This is the dictionary of 128 words used by print_cstr.
#  0 - to		32 - abort		64 - internal		 96 - support
#  1 - location		33 - you		65 - complete		 97 - write
#  2 - program		34 - the		66 - an			 98 - up
#  3 - memory		35 - is			67 - header		 99 - stack
#  4 - hex		36 - and		68 - register		100 - press
#  5 - unexpected	37 - interrupt		69 - must		101 - see
#  6 - run		38 - in			70 - line		102 - reset
#  7 - new		39 - be			71 - found		103 - pointer
#  8 - jump		40 - with		72 - quit		104 - fixed
#  9 - file		41 - as			73 - type		105 - detection
# 10 - download		42 - code		74 - which		106 - may
# 11 - bytes		43 - will		75 - erase		107 - has
# 12 - esc		44 - from		76 - step		108 - assemble
# 13 - information	45 - that		77 - provide		109 - clear
# 14 - this		46 - at			78 - so			110 - configure
# 15 - start		47 - used		79 - single		111 - data
# 16 - rom		48 - if			80 - should		112 - change
# 17 - receive		49 - by			81 - list		113 - allow
# 18 - ram		50 - value		82 - search		114 - written
# 19 - upload		51 - not		83 - eprom		115 - interface
# 20 - paulmon		52 - for		84 - next		116 - install
# 21 - or		53 - baud		85 - more		117 - checksum
# 22 - of		54 - when		86 - available		118 - instruction
# 23 - no		55 - rate		87 - help		119 - unchanged
# 24 - intel		56 - can		88 - edit		120 - end
# 25 - flash		57 - are		89 - well		121 - transfer
# 26 - external		58 - use		90 - user		122 - time
# 27 - errors		59 - serial		91 - dump		123 - any
# 28 - editing		60 - auto		92 - delays		124 - skip
# 29 - digits		61 - port		93 - these		125 - name
# 30 - command		62 - all		94 - terminal		126 - address
# 31 - begin		63 - make		95 - system		127 - print
common_words:
	dc.b	0x82, 0x90, 0xE8, 0x23, 0x86, 0x05, 0x4C, 0xF8
	dc.b	0x44, 0xB3, 0xB0, 0xB1, 0x48, 0x5F, 0xF0, 0x11
	dc.b	0x7F, 0xA0, 0x15, 0x7F, 0x1C, 0x2E, 0xD1, 0x40
	dc.b	0x5A, 0x50, 0xF1, 0x03, 0xBF, 0xBA, 0x0C, 0x2F
	dc.b	0x96, 0x01, 0x8D, 0x3F, 0x95, 0x38, 0x0D, 0x6F
	dc.b	0x5F, 0x12, 0x07, 0x71, 0x0E, 0x56, 0x2F, 0x48
	dc.b	0x3B, 0x62, 0x58, 0x20, 0x1F, 0x76, 0x70, 0x32
	dc.b	0x24, 0x40, 0xB8, 0x40, 0xE1, 0x61, 0x8F, 0x01
	dc.b	0x34, 0x0B, 0xCA, 0x89, 0xD3, 0xC0, 0xA3, 0xB9
	dc.b	0x58, 0x80, 0x04, 0xF8, 0x02, 0x85, 0x60, 0x25
	dc.b	0x91, 0xF0, 0x92, 0x73, 0x1F, 0x10, 0x7F, 0x12
	dc.b	0x54, 0x93, 0x10, 0x44, 0x48, 0x07, 0xD1, 0x26
	dc.b	0x56, 0x4F, 0xD0, 0xF6, 0x64, 0x72, 0xE0, 0xB8
	dc.b	0x3B, 0xD5, 0xF0, 0x16, 0x4F, 0x56, 0x30, 0x6F
	dc.b	0x48, 0x02, 0x5F, 0xA8, 0x20, 0x1F, 0x01, 0x76
	dc.b	0x30, 0xD5, 0x60, 0x25, 0x41, 0xA4, 0x2C, 0x60
	dc.b	0x05, 0x6F, 0x01, 0x3F, 0x26, 0x1F, 0x30, 0x07
	dc.b	0x8E, 0x1D, 0xF0, 0x63, 0x99, 0xF0, 0x42, 0xB8
	dc.b	0x20, 0x1F, 0x23, 0x30, 0x02, 0x7A, 0xD1, 0x60
	dc.b	0x2F, 0xF0, 0xF6, 0x05, 0x8F, 0x93, 0x1A, 0x50
	dc.b	0x28, 0xF0, 0x82, 0x04, 0x6F, 0xA3, 0x0D, 0x3F
	dc.b	0x1F, 0x51, 0x40, 0x23, 0x01, 0x3E, 0x05, 0x43
	dc.b	0x01, 0x7A, 0x01, 0x17, 0x64, 0x93, 0x30, 0x2A
	dc.b	0x08, 0x8C, 0x24, 0x30, 0x99, 0xB0, 0xF3, 0x19
	dc.b	0x60, 0x25, 0x41, 0x35, 0x09, 0x8E, 0xCB, 0x19
	dc.b	0x12, 0x30, 0x05, 0x1F, 0x31, 0x1D, 0x04, 0x14
	dc.b	0x4F, 0x76, 0x12, 0x04, 0xAB, 0x27, 0x90, 0x56
	dc.b	0x01, 0x2F, 0xA8, 0xD5, 0xF0, 0xAA, 0x26, 0x20
	dc.b	0x5F, 0x1C, 0xF0, 0xF3, 0x61, 0xFE, 0x01, 0x41
	dc.b	0x73, 0x01, 0x27, 0xC1, 0xC0, 0x84, 0x8F, 0xD6
	dc.b	0x01, 0x87, 0x70, 0x56, 0x4F, 0x19, 0x70, 0x1F
	dc.b	0xA8, 0xD9, 0x90, 0x76, 0x02, 0x17, 0x43, 0xFE
	dc.b	0x01, 0xC1, 0x84, 0x0B, 0x15, 0x7F, 0x02, 0x8B
	dc.b	0x14, 0x30, 0x8F, 0x63, 0x39, 0x6F, 0x19, 0xF0
	dc.b	0x11, 0xC9, 0x10, 0x6D, 0x02, 0x3F, 0x91, 0x09
	dc.b	0x7A, 0x41, 0xD0, 0xBA, 0x0C, 0x1D, 0x39, 0x5F
	dc.b	0x07, 0xF2, 0x11, 0x17, 0x20, 0x41, 0x6B, 0x35
	dc.b	0x09, 0xF7, 0x75, 0x12, 0x0B, 0xA7, 0xCC, 0x48
	dc.b	0x02, 0x3F, 0x64, 0x12, 0xA0, 0x0C, 0x27, 0xE3
	dc.b	0x9F, 0xC0, 0x14, 0x77, 0x70, 0x11, 0x40, 0x71
	dc.b	0x21, 0xC0, 0x68, 0x25, 0x41, 0xF0, 0x62, 0x7F
	dc.b	0xD1, 0xD0, 0x21, 0xE1, 0x62, 0x58, 0xB0, 0xF3
	dc.b	0x05, 0x1F, 0x73, 0x30, 0x77, 0xB1, 0x6F, 0x19
	dc.b	0xE0, 0x19, 0x43, 0xE0, 0x58, 0x2F, 0xF6, 0xA4
	dc.b	0x14, 0xD0, 0x23, 0x03, 0xFE, 0x31, 0xF5, 0x14
	dc.b	0x30, 0x99, 0xF8, 0x03, 0x3F, 0x64, 0x22, 0x51
	dc.b	0x60, 0x25, 0x41, 0x2F, 0xE3, 0x01, 0x56, 0x27
	dc.b	0x93, 0x09, 0xFE, 0x11, 0xFE, 0x79, 0xBA, 0x60
	dc.b	0x75, 0x42, 0xEA, 0x62, 0x58, 0xA0, 0xE5, 0x1F
	dc.b	0x53, 0x4F, 0xD1, 0x10, 0xd5, 0x20, 0x34, 0x75
	dc.b	0x2f, 0x41, 0x20, 0xb6, 0x01, 0x53, 0x5f, 0x70
	dc.b	0x9f, 0xc6, 0x50, 0xb3, 0x01, 0xd3, 0x4d, 0x71
	dc.b	0x07, 0x4c, 0x56, 0x02

# List of strings
#################################
# Strings used to dump register contents
str_reg_sp:			.asciz	"SP: "
str_reg_sr:			.asciz	"SR: "

#str_logon1:			db	"Welcome",128," z80Mon v0.1",13,14			/* Welcome string (OLD) */
#str_logon2:	 		db	32,32,"See",148,"2.DOC,",148,"2.EQU",164
#				db	148,"2.HDR",180,213,141,".",14				/* Documentation string */
str_logon1:			.ascii	"Welcome"						/* Welcome string */
				dc.b	128
				.asciz	" m68kMon "
#str_prompt1:			db	148,"2 Loc:",0						/* Paulmon2 Loc: (OLD) */
str_prompt1:			.asciz	"m68kMon:"						/* m68kMon: */
str_prompt2:			dc.b	' ','>',160						/*  > abort run which program(	(must follow after prompt1) */
#str_prompt3:			db	134,202,130,'(',0					/* run which program( */
#str_prompt4:			db	"),",149,140,128,200,": ",0				/* ), or esc to quit: (OLD) */
str_prompt4:			dc.b	',',149,31,140,':',' ',0				/* , or Escape: */
#str_prompt5:			db	31,151,130,195,"s",199,166,131,","
#				db	186," JUMP",128,134,161,"r",130,13,14			/* No program headers found in memory, use JUMP to run your program */
str_prompt6:			dc.b	13,13,31,135,131,129,':',' ',0				/* \n\nNew memory location: */
str_prompt7:			dc.b	31,228,251,' ','k','e','y',':',' ',0			/* Press any key: */
#str_prompt8:			db	13,13,31,136,128,131,129," (",0				/* \n\nJump to memory location ( (OLD) */
str_prompt8:			dc.b	13,13,31,136,128,131,129,0				/* \n\nJump to memory location */
#str_prompt9:			db	13,13,31,130,31,253,0					; \n\nProgram Name (OLD)
str_prompt9:			dc.b	13,31,130,31,253,0					/* \nProgram Name */
str_prompt9b:			dc.b	31,129,32,32,32,32,32,31,201,14				/* Location      Type	 (must follow prompt9) */
str_prompt10:			dc.b	')',' ',31,135,31,178,':',' ',0				/* ) New Value: */
str_prompt11:			dc.b	31,189,':',' ',0					/* Port: */
str_prompt12:			dc.b	31,178,':',' ',0					/* Value: */
str_prompt14:			dc.b	13,13,31,135,227,129,':',' ',0				/* \n\nNew stack location: */
str_prompt15:			dc.b	31,228,251						/* Press any key for next page, or esc to quit */
				.ascii	" key"
				dc.b	180,212
				.ascii	" page,"
				dc.b	149,140,128,200,14

str_type1:			dc.b	31,154,158,0						/* External command */
str_type2:			dc.b	31,130,0						/* Program */
str_type3:			.asciz	"Init"							/* Init */
str_type4:			dc.b	31,143,31,226,31,170,0					/* Start Up Code */
str_type5:			.asciz	"???"							/* ??? */

str_tag_help2:			dc.b	31,215,0						/* Help */
str_tag_help1: 			dc.b	31,142,215,209,0					/* This help list (these 11 _cmd string must be in order) */
str_tag_listm:  		dc.b	31,209,130,'s',0					/* List Programs */
#str_tag_run:  			dc.b	31,134,130,0						/* Run Program */
str_tag_dnld: 			dc.b	31,138,0						/* Download */
str_tag_upld: 			dc.b	31,147,0						/* Upload */
str_tag_nloc: 			dc.b	31,135,129,0						/* New Location */
str_tag_jump: 			dc.b	31,136,128,131,129,0					/* Jump to memory location */
#str_tag_dump: 			dc.b	31,132,219,154,131,0					/* Hex dump external memory (OLD) */
str_tag_hexdump: 		dc.b	31,132,219,131,0					/* Hex dump memory */
str_tag_in:			.ascii	"Read"							/* Read in port */
				dc.b	166,189,0
str_tag_out:			dc.b	31,225,128,189,0					/* Write to port */
str_tag_regdump: 		dc.b	31,219,31,196,'s',0					/* Dump Registers */
#str_tag_edit: 			dc.b	31,156,154,146,0					/* Editing external ram (OLD) */
str_tag_edit: 			dc.b	31,216,31,146,0						/* Edit Ram */
str_tag_clrmem: 		dc.b	31,237,131,0						/* Clear memory */
str_tag_stack:			dc.b	31,240,227,129,0					/* Change stack location */

str_help1:			dc.b	13,13							/* \n\nStandard Commands */
				.ascii	"Standard"
				dc.b	31,158,'s',14
str_help2:			dc.b	31,218,31,244,'e','d',31,158,'s',14			/* User Installed Commands */
#str_abort:			dc.b	' ',31,158,31,160,'!',13,14				/*  Command Abort!\n\n */
str_abort:			dc.b	13,' ',31,158,31,160,'!',14				/* \n Command Abort!\n */
#str_runs:			dc.b	13,134,'n','i','n','g',130,':',13,14			/* \nRunning program:\n\n */
str_runs:			dc.b	13,134,'n','i','n','g',130,' ','@',0			/* \nRunning program @ */

#str_edit1: 			dc.b	13,13,31,156,154,146,',',140,128,200,14			/* \n\nEditing external ram, esc to quit\n (OLD) */
str_edit1: 			dc.b	13,13,31,156,31,146,',',31,140,128,200,14		/* \n\nEditing Ram, Esc to quit\n */
str_edit2: 			dc.b	' ',' ',31,156,193,',',142,129,247,13,14		/*   Editing complete, this location unchanged\n\n */

str_start_addr:			dc.b	31,143,31,254,':',' ',0
str_end_addr:			dc.b	31,248,31,254,':',' ',0
str_sure:			dc.b	31,185,161,' ','s','u','r','e','?',' ',0		/* Are you sure?  */
str_clrcomp:			dc.b	31,131,237,193,14					/* Memory clear complete\n */

str_invalid:			.ascii	"Invalid selection"
				dc.b	14

str_upld1: 			dc.b	13,13							/* \n\nSending SREC hex file from */
				.ascii	"Sending SREC"
				dc.b	137,172,32,32,0
str_upld2:			dc.b	' ','t','o',' ',0					/* to */
#str_upld2: 			dc.b	' ',128,32,32,0						/*  to */
str_upld_srec_hdr:		.asciz	"m68kMon SREC"

str_dnld1: 			dc.b	13,13,31,159						/* \n\nBegin ascii transfer of SREC hex file */
				.ascii	" ascii"
				dc.b	249,150,31,' ','S','R','E','C',132,137
				dc.b	',',149,140,128,160,13,14				/* , or esc to abort \n\n */
str_dnld2: 			dc.b	13,31,138,160,'e','d',13,14				/* \nDownload aborted\n\n */
str_dnld3: 			dc.b	13,31,138,193,'d',13,14					/* \nDownload completed\n\n */
str_dnld4: 			.ascii	"Summary:"						/* Summary:\n */
				dc.b	14
str_dnld5: 			dc.b	' ',198,'s',145,'d',14					/*  lines received\n */
str_dnld6a:			dc.b	' ',139,145,'d',14					/*  bytes received\n */
str_dnld6b:			dc.b	' ',139							/*  bytes written\n */
				.ascii	" written"
				dc.b	14
str_dnld7: 			dc.b	31,155,':',14						/* Errors:\n */
str_dnld8: 			dc.b	' ',139							/*  bytes unable to write\n */
				.ascii	" unable"
				dc.b	128
				.ascii	" write"
				dc.b	14
str_dnld9: 			dc.b	32,32,'b','a','d',245,'s',14				/*  bad checksums\n */
str_dnld10:			dc.b	' ',133,159,150,198,14					/*  unexpected begin of line\n */
str_dnld11:			dc.b	' ',133,' ','n','o','n',132,157,14			/*  unexpected non hex digits\n */
str_dnld12:			.asciz	" wrong byte count\n"					/*  wrong byte count\n */
str_dnld13:			dc.b	31,151,155						/* No errors detected\n\n */
				.ascii	" detected"
				dc.b	13,14
str_recving:			.asciz	"Receiving: "

str_ny:				.asciz	" (N/y): "
str_version:			.asciz	"Version: "

str_exception_ga:		.asciz	"Guru Meditation"
.equiv				str_exception_ga_size, (. - str_exception_ga -1)
str_exception_address:		.asciz	"Exception: Address error"
str_exception_bus:		.asciz	"Exception: Bus error"
str_exception_interrupt:	.asciz	"Exception: Interrupt"
str_exception_unknown:		.asciz	"Exception: Unknown"
str_exception_zero:		.asciz	"Exception: Divide By Zero"
