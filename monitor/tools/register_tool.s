###########################################################################
#                                                                         #
#       Tool to read/write memory locations (spefically registers)        #
#                                                                         #
###########################################################################

.include "../cisco-2500/scn2681.h"
.equiv	console_out, scn2681_out_A
.equiv	console_in, scn2681_in_A

.org	0x0
	dc.b	0xA5,0xE5,0xE0,0xA5                                     /* signiture bytes */
	dc.b	254,'!',0,0                                             /* id (254=cmd) */
	dc.b	0,0,0,0                                                 /* prompt code vector */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* user defined */
	dc.b	255,255,255,255                                         /* length and checksum (255=unused) */
	.asciz	"Register Tool"

.org	0x40
/* Main loop */
register_tool:
	/* Print banner */
	jsr	print_newline
	lea	str_banner, %a0
	jsr	print_str_simple
	jsr	print_newline

	/* Configure bus error handler */
	movea.l #8, %a0							/* Address of bus error exception handler in EVT */
	lea.l   register_tool_bus_error_handler, %a1			/* Address of new bus error exception handler */
	move.l  %a1, (%a0)						/* Save new exception handler */

register_tool_loop:
	lea	str_action, %a0						/* Print action string */
	jsr	print_str_simple
	lea	register_tool_loop, %a0					/* Push return address onto stack */
	move.l	%a0, -(%a7)
	lea	flag_bus_error,	%a0					/* Get address of bus error flag */
	move.b	#0x0, (%a0)						/* Clear bus error flag */
register_tool_action_loop:
	jsr	console_in						/* Read action character in */
	jsr	char_2_upper						/* Convert to upper to simplify matching */
	cmp.b	#'R', %d0
	beq.w	register_tool_read
	cmp.b	#'W', %d0
	beq.w	register_tool_write
	cmp.b	#'A', %d0
	beq.w	register_tool_and
	cmp.b	#'O', %d0
	beq.w	register_tool_or
	cmp.b	#'Q', %d0
	beq.w	register_tool_quit
	bra.s	register_tool_action_loop

register_tool_quit:
	lea	str_action_quit, %a0					/* Print action */
	jsr	print_str_simple
	jsr	print_newline
	move.l	(%a7)+, %d0						/* Pop 'register_tool_loop' address off stack */
	rts

# register_tool_abort
#################################
# Prints 'Canceled'
register_tool_abort:
	jsr	print_space
	lea	str_canceled, %a0
	jsr	print_str_simple
	jsr	print_newline
	jsr	print_newline
	rts

# print_str_fake
#################################
#  Prints spaces instead of characters for a null terminated string.
#  Retains the ability ability to print consecutive strings.
#       In:     A0 = Pointer to string
print_str_fake:
	move.b	#' ', %d0
print_str_fake_loop:
	tst.b   (%a0)+							/* Check next character and increment pointer */
	beq.b   print_str_fake_end					/* Check whether NULL character */
	jsr     console_out						/* Print space */
	bra.s   print_str_fake_loop					/* Loop */
print_str_fake_end:
	rts

# print_sr_flags
#################################
#  Print a representation of the Status Register flags
#	In:	D3 = Status register contents
print_sr_flags:
	lea	str_sr, %a0
	jsr	print_str_simple
	move.w	%d3, %d2
	jsr	print_hex16
	jsr	print_space
	move.b	#'[', %d0
	jsr	console_out
	lea	str_sr_flags, %a0
print_sr_flags_bitf:
	btst	#0xf, %d3
	bne.s	print_sr_flags_bitf_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bite
print_sr_flags_bitf_set:
	jsr	print_str_simple
print_sr_flags_bite:
	btst	#0xe, %d3
	bne.s	print_sr_flags_bite_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bitd
print_sr_flags_bite_set:
	jsr	print_str_simple
print_sr_flags_bitd:
	btst	#0xd, %d3
	bne.s	print_sr_flags_bitd_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bitc
print_sr_flags_bitd_set:
	jsr	print_str_simple
print_sr_flags_bitc:
	btst	#0xc, %d3
	bne.s	print_sr_flags_bitc_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bita
print_sr_flags_bitc_set:
	jsr	print_str_simple
#print_sr_flags_bitb:
#	btst	#0xb, %d3
#	bne.s	print_sr_flags_bitb_set
#	jsr	print_str_fake
#	bra.s	print_sr_flags_bita
#print_sr_flags_bitb_set:
#	jsr	print_str_simple
print_sr_flags_bita:
	btst	#0xa, %d3
	bne.s	print_sr_flags_bita_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit9
print_sr_flags_bita_set:
	jsr	print_str_simple
print_sr_flags_bit9:
	btst	#0x9, %d3
	bne.s	print_sr_flags_bit9_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit8
print_sr_flags_bit9_set:
	jsr	print_str_simple
print_sr_flags_bit8:
	btst	#0x8, %d3
	bne.s	print_sr_flags_bit8_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit4
print_sr_flags_bit8_set:
	jsr	print_str_simple
#print_sr_flags_bit7:
#	btst	#0x7, %d3
#	bne.s	print_sr_flags_bit7_set
#	jsr	print_str_fake
#	bra.s	print_sr_flags_bit6
#print_sr_flags_bit7_set:
#	jsr	print_str_simple
#print_sr_flags_bit6:
#	btst	#0x6, %d3
#	bne.s	print_sr_flags_bit6_set
#	jsr	print_str_fake
#	bra.s	print_sr_flags_bit5
#print_sr_flags_bit6_set:
#	jsr	print_str_simple
#print_sr_flags_bit5:
#	btst	#0x5, %d3
#	bne.s	print_sr_flags_bit5_set
#	jsr	print_str_fake
#	bra.s	print_sr_flags_bit4
#print_sr_flags_bit5_set:
#	jsr	print_str_simple
print_sr_flags_bit4:
	btst	#0x4, %d3
	bne.s	print_sr_flags_bit4_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit3
print_sr_flags_bit4_set:
	jsr	print_str_simple
print_sr_flags_bit3:
	btst	#0x3, %d3
	bne.s	print_sr_flags_bit3_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit2
print_sr_flags_bit3_set:
	jsr	print_str_simple
print_sr_flags_bit2:
	btst	#0x2, %d3
	bne.s	print_sr_flags_bit2_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit1
print_sr_flags_bit2_set:
	jsr	print_str_simple
print_sr_flags_bit1:
	btst	#0x1, %d3
	bne.s	print_sr_flags_bit1_set
	jsr	print_str_fake
	bra.s	print_sr_flags_bit0
print_sr_flags_bit1_set:
	jsr	print_str_simple
print_sr_flags_bit0:
	btst	#0x0, %d3
	bne.s	print_sr_flags_bit0_set
	jsr	print_str_fake
	bra.s	print_sr_flags_finish
print_sr_flags_bit0_set:
	jsr	print_str_simple
print_sr_flags_finish:
	move.b	#']', %d0
	jsr	console_out
	rts

# print_bus_error
#################################
#  Print 'Bus Error'
print_bus_error:
	jsr	print_space
	lea	str_bus_error, %a0
	jsr	print_str_simple
	jsr	print_newline
	jsr	print_newline
	rts

# register_tool_bus_error_handler
#################################
#  Bus error handler for this tool
register_tool_bus_error_handler:
	move.l	%a0, -(%a7)						/* Save A0 */
	lea	flag_bus_error, %a0					/* Get address of bus error flag */
	move.b	#0xff, (%a0)						/* Set bus error flag */
	move.l	(%a7)+, %a0						/* Restore A0 */
	andi.w  #0xceff, (0xa, %a7)					/* Clear re-run flags */
	rte

# Read action
###########################################################################
register_tool_read:
	lea	str_action_read, %a0					/* Print action */
	jsr	print_str_simple
	jsr	print_newline

	lea	str_action_read, %a0					/* Request address */
	jsr	print_str_simple
	jsr	print_space
	lea	str_mem_address, %a0
	jsr	print_str_simple
	jsr	input_hex32						/* Get address */
	bcc.w	register_tool_abort
	move.l	%d0, %d4						/* Save address */

	jsr	print_newline
	lea	str_action_read, %a0					/* Request operation width */
	jsr	print_str_simple
	jsr	print_space
	lea	str_opwidth, %a0
	jsr	print_str_simple
register_tool_read_opwidth_loop:
	jsr	console_in						/* Get operation width */
	jsr	char_2_upper						/* Convert to upper to simplify matching */
	cmp.b	#'B', %d0
	beq.s	register_tool_read_byte
	cmp.b	#'W', %d0
	beq.s	register_tool_read_word
	cmp.b	#'L', %d0
	beq.w	register_tool_read_long
	cmp.b	#0x1b, %d0						/* Escape */
	beq.w	register_tool_abort
	bra.s	register_tool_read_opwidth_loop

register_tool_read_byte:
	lea	str_opwidth_byte, %a0
	jsr	print_str_simple
	jsr	print_newline
	move.l	%d4, %d3						/* Print address */
	jsr	print_hex32
	jsr	print_colon_space
	move.l	%d4, %a0
	move.b	(%a0), %d1						/* Print data at address */
	move	%sr, %d5						/* Save status flags */
	lea	flag_bus_error, %a0					/* Get address of bus error flag */
	tst.b	(%a0)							/* Check flag state */
	bne.w	print_bus_error
	jsr	print_hex8
	jsr	print_newline
	move.w	%d5, %d3
	jsr	print_sr_flags
	jsr	print_newline
	jsr	print_newline
	rts

register_tool_read_word:
	lea	str_opwidth_word, %a0
	jsr	print_str_simple
	jsr	print_newline
	move.l	%d4, %d3						/* Print address */
	jsr	print_hex32
	jsr	print_colon_space
	move.l	%d4, %a0
	move.w	(%a0), %d2						/* Print data at address */
	move	%sr, %d5						/* Save status flags */
	lea	flag_bus_error, %a0					/* Get address of bus error flag */
	tst.b	(%a0)							/* Check flag state */
	bne.w	print_bus_error
	jsr	print_hex16
	jsr	print_newline
	move.w	%d5, %d3
	jsr	print_sr_flags
	jsr	print_newline
	jsr	print_newline
	rts

register_tool_read_long:
	lea	str_opwidth_long, %a0
	jsr	print_str_simple
	jsr	print_newline
	move.l	%d4, %d3						/* Print address */
	jsr	print_hex32
	jsr	print_colon_space
	move.l	%d4, %a0
	move.l	(%a0), %d3						/* Print data at address */
	move	%sr, %d5						/* Save status flags */
	lea	flag_bus_error, %a0					/* Get address of bus error flag */
	tst.b	(%a0)							/* Check flag state */
	bne.w	print_bus_error
	jsr	print_hex32
	jsr	print_newline
	move.w	%d5, %d3
	jsr	print_sr_flags
	jsr	print_newline
	jsr	print_newline
	rts

# Write action
###########################################################################
register_tool_write:
	lea	str_action_write, %a0					/* Print action */
	jsr	print_str_simple
	jsr	print_newline

	rts

# AND action
###########################################################################
register_tool_and:
	lea	str_action_and, %a0					/* Print action */
	jsr	print_str_simple
	jsr	print_newline

	rts

# OR action
###########################################################################
register_tool_or:
	lea	str_action_or, %a0					/* Print action */
	jsr	print_str_simple
	jsr	print_newline

	rts

.org	0x400
flag_bus_error:		dc.b		0x00
str_action:		.asciz		"Action [(R)ead/(W)rite/(A)nd/(O)r or (Q)uit]: "
str_action_read:	.asciz		"Read"
str_action_write:	.asciz		"Write"
str_action_and:		.asciz		"And"
str_action_or:		.asciz		"Or"
str_action_quit:	.asciz		"Quit"
str_banner:		.asciz		"   Register Tool   \n\r==================="
str_bus_error:		.asciz		"Bus Error"
str_canceled:		.asciz		"Canceled"
str_mem_address:	.asciz		"Memory Address: 0x"
str_opwidth:		.asciz		"Width [(B)yte/(W)ord/(L)ong]: "
str_opwidth_byte:	.asciz		"Byte"
str_opwidth_long:	.asciz		"Long"
str_opwidth_word:	.asciz		"Word"
str_sr:			.asciz		"Status Register: "
str_sr_flags:		.asciz		"T1"
			.asciz		"T0"
			.asciz		"S"
			.asciz		"M"
			.asciz		"I2"
			.asciz		"I1"
			.asciz		"I0"
			.asciz		"X"
			.asciz		"N"
			.asciz		"Z"
			.asciz		"V"
			.asciz		"C"
str_value:		.asciz		"Value: 0x"

.org	0x500
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
	jsr	string_2_hex16						/* Read first word */
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
	jsr	string_2_hex8						/* Read first byte */
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

# print_abort
#################################
#  Print the abort string
print_abort:
	lea	str_abort, %a0
	jmp	print_cstr

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

	cmp.b	#0x1b, %d0						/* Check whether character is escape key */
	beq.s	input_hex_abort
	cmp.b	#0x08, %d0						/* Check whether character is backspace key */
	beq.s	input_hex_delete_digit
	cmp.b	#0x7f, %d0						/* Check whether character is delete key */
	beq.s	input_hex_delete_digit
	cmp.b	#0x0d, %d0						/* Check whether character is CR key */
	beq.s	input_hex_complete
	cmp.b	#0x0a, %d0						/* Check whether character is CR key */
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
	mov.b	#0x08, %d0						/* Back 1 character again */
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

	cmp.b	#0x1b, %d0						/* Check whether character is escape key */
	beq.s	input_str_abort
	cmp.b	#0x08, %d0						/* Check whether character is backspace key */
	beq.s	input_str_delete_char
	cmp.b	#0x7f, %d0						/* Check whether character is delete key */
	beq.s	input_str_delete_char
	cmp.b	#0x0d, %d0						/* Check whether character is CR key */
	beq.s	input_str_complete
	cmp.b	#0x0a, %d0						/* Check whether character is CR key */
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
	mov.b	#0x08, %d0
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


