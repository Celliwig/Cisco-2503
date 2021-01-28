###########################################################################
#                                                                         #
#       Tool to interface with emulator to access system registers        #
#                                                                         #
###########################################################################
#
# System Register Integrator:
#	This tool runs on the hardware and interfaces with the emulator
#	to provide real values from the hardware to the emulator while
#	 running IOS.
#
# Protocol:
#	Read Request:
#		RAAAAAAAASS
#			R - Read Header
#			AAAAAAAA - Address
#			SS - Num. bytes (01/02/04)
#	Read Response:
#		RAAAAAAAA01BEVV
#		RAAAAAAAA02BEVVVV
#		RAAAAAAAA04BEVVVVVVVV
#			R - Read Header
#			AAAAAAAA - Address
#			SS - Num. bytes (01/02/04)
#			BE - 00 = No Bus Error / xx = Bus Error
#			VVn - Value read
#	Write Request:
#		WAAAAAAAA02VV
#		WAAAAAAAA04VVVV
#		WAAAAAAAA08VVVVVVVV
#			W - Header
#			AAAAAAAA - Address
#			SS - Num. bytes (01/02/04)
#			VVn - Value to write
#	Write Response:
#		WAAAAAAAA01BEVV
#		WAAAAAAAA02BEVVVV
#		WAAAAAAAA04BEVVVVVVVV
#			W - Header
#			AAAAAAAA - Address
#			SS - Num. bytes (01/02/04)
#			BE - 00 = No Bus Error / xx = Bus Error
#			VVn - Value written

.include "../cisco-2500/scn2681.h"
.equiv	console_out, scn2681_out_A
.equiv	console_in, scn2681_in_A

.equiv 	buffer_str_size, 0x20

.org	0x0
	dc.b	0xA5,0xE5,0xE0,0xA5                                     /* signiture bytes */
	dc.b	254,'!',0,0                                             /* id (254=cmd) */
	dc.b	0,0,0,0                                                 /* prompt code vector */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* user defined */
	dc.b	255,255,255,255                                         /* length and checksum (255=unused) */
	.asciz	"SRI Tool"

.org	0x40
/* Main loop */
sri_tool:
	/* Configure bus error handler */
	movea.l #8, %a0							/* Address of bus error exception handler in EVT */
	lea.l   sri_tool_bus_error_handler, %a1				/* Address of new bus error exception handler */
	move.l  %a1, (%a0)						/* Save new exception handler */

sri_tool_loop:
	lea	sri_tool_loop, %a0					/* Push return address onto stack */
	move.l	%a0, -(%a7)
	lea	flag_bus_error,	%a0					/* Get address of bus error flag */
	move.b	#0x0, (%a0)						/* Clear bus error flag */
sri_tool_cmd_init:
	lea	buffer_str, %a0						/* Load pointer to string buffer */
	clr.l	%d4							/* D4 is character count */
sri_tool_cmd_loop:
	jsr	console_in						/* Get character */
	add.l	#0x1, %d4						/* Increment character count */
	cmp.b	#'\n', %d0						/* Check for End Of Line */
	beq.s	sri_tool_cmd_process					/* Process command */
	cmp.b	#'\r', %d0						/* Check for End Of Line */
	beq.s	sri_tool_cmd_process					/* Process command */
	cmp.l	#buffer_str_size, %d4					/* Check if space in buffer */
	bgt.s	sri_tool_cmd_loop					/* Run out of space in the buffer, just loop */
sri_tool_cmd_store_char:
	move.b	%d0, (%a0)+						/* Store character in buffer */
	bra.s	sri_tool_cmd_loop					/* Loop for more characters */

sri_tool_cmd_process:
	cmp.b	#0x01, %d4						/* Check if this is just a carriage return/newline */
	beq.s	sri_tool_cmd_init
	lea	buffer_str, %a0						/* Load pointer to string buffer */
	move.b	(%a0)+, %d0						/* Get command character */
	jsr	char_2_upper						/* Convert to upper to simplify matching */
	cmp.b	#'R', %d0						/* Check for READ command */
	beq.s	sri_tool_read_cmd
	cmp.b	#'W', %d0						/* Check for WRITE command */
	beq.w	sri_tool_write_cmd
	cmp.b	#'Q', %d0						/* Check for QUIT command */
	beq.s	sri_tool_quit
	bra.s	sri_tool_error

sri_tool_quit:
	jsr	print_newline
	move.l	(%a7)+, %d0						/* Pop 'register_tool_loop' address off stack */
	rts

sri_tool_error:
	jsr	print_newline
	lea	str_error, %a0
	jsr	print_str_simple
	rts

# sri_tool_bus_error_handler
#################################
#  Bus error handler for this tool
sri_tool_bus_error_handler:
	move.l	%a0, -(%a7)						/* Save A0 */
	lea	flag_bus_error, %a0					/* Get address of bus error flag */
	move.b	#0xff, (%a0)						/* Set bus error flag */
	move.l	(%a7)+, %a0						/* Restore A0 */
	andi.w  #0xceff, (0xa, %a7)					/* Clear re-run flags */
	rte

# Read command
###########################################################################
sri_tool_read_cmd:
	cmp.b	#0xd, %d4						/* Check byte count (upto 13 chars with '\r\n') */
	bgt.s	sri_tool_error

	jsr	string_2_hex32						/* Get address */
	bcc.s	sri_tool_error
	move.l	%d1, %d5						/* Store address */

	jsr	string_2_hex8						/* Num. bytes */
	bcc.s	sri_tool_error
	cmp.b	#0x1, %d1
	beq.s	sri_tool_read_cmd_byte
	cmp.b	#0x2, %d1
	beq.s	sri_tool_read_cmd_word
	cmp.b	#0x4, %d1
	beq.s	sri_tool_read_cmd_long
	bra.s	sri_tool_error

sri_tool_read_cmd_byte:
	move.b	#'R', %d0						/* Print header */
	jsr	console_out
	move.l	%d5, %d3						/* Get address */
	jsr	print_hex32						/* Print address */
	move.b	#0x01, %d1
	jsr	print_hex8						/* Print byte count */
	move.l	%d5, %a0
	move.b	(%a0), %d6						/* Get requested value */
	lea	flag_bus_error, %a0
	move.b	(%a0), %d1
	jsr	print_hex8						/* Print bus error flag status */
	move.b	%d6, %d1
	jsr	print_hex8						/* Print requested value */
	jsr	print_newline
	rts

sri_tool_read_cmd_word:
	move.b	#'R', %d0						/* Print header */
	jsr	console_out
	move.l	%d5, %d3						/* Get address */
	jsr	print_hex32						/* Print address */
	move.b	#0x02, %d1
	jsr	print_hex8						/* Print byte count */
	move.l	%d5, %a0
	move.w	(%a0), %d6						/* Get requested value */
	lea	flag_bus_error, %a0
	move.b	(%a0), %d1
	jsr	print_hex8						/* Print bus error flag status */
	move.w	%d6, %d2
	jsr	print_hex16						/* Print requested value */
	jsr	print_newline
	rts

sri_tool_read_cmd_long:
	move.b	#'R', %d0						/* Print header */
	jsr	console_out
	move.l	%d5, %d3						/* Get address */
	jsr	print_hex32						/* Print address */
	move.b	#0x04, %d1
	jsr	print_hex8						/* Print byte count */
	move.l	%d5, %a0
	move.l	(%a0), %d6						/* Get requested value */
	lea	flag_bus_error, %a0
	move.b	(%a0), %d1
	jsr	print_hex8						/* Print bus error flag status */
	move.l	%d6, %d3
	jsr	print_hex32						/* Print requested value */
	jsr	print_newline
	rts

# Write command
###########################################################################
sri_tool_write_cmd:
	cmp.b	#0x15, %d4						/* Check byte count (upto 21 chars with '\r\n') */
	bgt.w	sri_tool_error

	jsr	string_2_hex32						/* Get address */
	bcc.w	sri_tool_error
	move.l	%d1, %d5						/* Store address */

	jsr	string_2_hex8						/* Num. bytes */
	bcc.w	sri_tool_error
	cmp.b	#0x1, %d1
	beq.s	sri_tool_write_cmd_byte
	cmp.b	#0x2, %d1
	beq.s	sri_tool_write_cmd_word
	cmp.b	#0x4, %d1
	beq.w	sri_tool_write_cmd_long
	bra.w	sri_tool_error

sri_tool_write_cmd_byte:
	jsr	string_2_hex8						/* Get data (byte) */
	bcc.w	sri_tool_error
	move.l	%d1, %d6						/* Store data */

	move.b	#'W', %d0						/* Print header */
	jsr	console_out
	move.l	%d5, %d3						/* Get address */
	jsr	print_hex32						/* Print address */
	move.b	#0x01, %d1
	jsr	print_hex8						/* Print byte count */
	move.l	%d5, %a0
	move.b	%d6, (%a0)						/* Write data */
	lea	flag_bus_error, %a0
	move.b	(%a0), %d1
	jsr	print_hex8						/* Print bus error flag status */
	move.l	%d6, %d1
	jsr	print_hex8						/* Print requested value */
	jsr	print_newline
	rts

sri_tool_write_cmd_word:
	jsr	string_2_hex16						/* Get data (word) */
	bcc.w	sri_tool_error
	move.l	%d1, %d6						/* Store data */

	move.b	#'W', %d0						/* Print header */
	jsr	console_out
	move.l	%d5, %d3						/* Get address */
	jsr	print_hex32						/* Print address */
	move.b	#0x02, %d1
	jsr	print_hex8						/* Print byte count */
	move.l	%d5, %a0
	move.w	%d6, (%a0)						/* Write data */
	lea	flag_bus_error, %a0
	move.b	(%a0), %d1
	jsr	print_hex8						/* Print bus error flag status */
	move.l	%d6, %d2
	jsr	print_hex16						/* Print requested value */
	jsr	print_newline
	rts

sri_tool_write_cmd_long:
	jsr	string_2_hex32						/* Get data (byte) */
	bcc.w	sri_tool_error
	move.l	%d1, %d6						/* Store data */

	move.b	#'W', %d0						/* Print header */
	jsr	console_out
	move.l	%d5, %d3						/* Get address */
	jsr	print_hex32						/* Print address */
	move.b	#0x04, %d1
	jsr	print_hex8						/* Print byte count */
	move.l	%d5, %a0
	move.l	%d6, (%a0)						/* Write data */
	lea	flag_bus_error, %a0
	move.b	(%a0), %d1
	jsr	print_hex8						/* Print bus error flag status */
	move.l	%d6, %d3
	jsr	print_hex32						/* Print requested value */
	jsr	print_newline
	rts

.org	0x280
buffer_str:		ds.b		buffer_str_size, 0x00;		/* Receive string buffer */
flag_bus_error:		dc.b		0x00				/* Flag indicating bus error occured */
str_error:		.asciz		"ERROR\r\n"

.org	0x2b0
######################################################################################################################################################
# From m68kmon.s
######################################################################################################################################################
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

# Print routines
###########################################################################

# print_newlinex2
#################################
#  Print 2 new lines
print_newlinex2:
	jsr	print_newline
# print_newline
#################################
#  Print a new line
print_newline:
	mov.b	#'\r', %d0
	jsr	console_out
	mov.b	#'\n', %d0
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

######################################################################################################################################################
# Copy of scn2681_io.s
######################################################################################################################################################
# scn2681_clear_errors_A
####################################################
#  Clear status errors
scn2681_clear_errors_A:
	mov.b	#SCN2681_REG_COMMAND_MISC_RESET_ERR, %d0
	mov.b	%d0, SCN2681_ADDR_WR_COMMAND_A
	rts

# Polling routines
###########################################################################
# scn2681_out_A
####################################################
# Writes a byte to console port (Blocking)
#	In:	D0 = Character byte
scn2681_out_A:
	btst	#2, SCN2681_ADDR_RD_STATUS_A		/* Check if transmitter ready bit is set */
	beq	scn2681_out_A
	move.b	%d0, SCN2681_ADDR_WR_TX_A		/* Transmit Character */
	rts

# scn2681_in_A_check
####################################################
#  Check whether character available in console buffer
#	Out:	Status set
scn2681_in_A_check:
	btst	#0, SCN2681_ADDR_RD_STATUS_A		/* Check if receiver ready bit is set */
	rts

# scn2681_in_A
####################################################
#  Reads a byte from console port (Blocking)
#	Out:	D0 = Character byte
scn2681_in_A:
	btst	#0, SCN2681_ADDR_RD_STATUS_A		/* Check if receiver ready bit is set */
	beq	scn2681_in_A
scn2681_in_A_nocheck:
	move.b	SCN2681_ADDR_RD_RX_A, %d0		/* Read Character into D0 */
	rts


