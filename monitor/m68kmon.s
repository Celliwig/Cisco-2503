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
.equiv	console_out, scn2681_out_A
###########################################################################

# Include definitions
.include "cisco-2500/cisco-2500.h"
.include "include/stddefs.h"

*; # Subroutine Jump Table
*; ###########################################################################
*orgmem	mem_base+0x80
*
*monlib_console_out:
*	jp	0x0010
*monlib_console_in:
*	jp	0x0010
*monlib_print_newline:
*	jp	print_newline
*monlib_print_hex_digit:
*	jp	print_hex_digit
*monlib_print_hex8:
*	jp	print_hex8
*monlib_print_hex16:
*	jp	print_hex16
*monlib_print_dec8u:
*	jp	print_dec8u
*monlib_print_dec8s:
*	jp	print_dec8s
*monlib_print_str:
*	jp	print_str
*monlib_char_2_upper:
*	jp	char_2_upper
*monlib_string_length:
*	jp	string_length
*
*;	ajmp	ghex
*;	ajmp	ghex16
*;	ajmp	esc
*;	ljmp	autobaud
*;pcstr_h:ljmp	pcstr
*;	ljmp	pint16u
*;	ljmp	module_find
*;input_character_filter_h:
*;	ljmp	input_character_filter
*;	ajmp	asc2hex
*
*; # Monitor Variables
*; ###########################################################################
*MONITOR_ADDR_CURRENT:
*	dw	0x1000
*z80mon_temp:
*z80mon_temp1:
*	dw	0x0000
*z80mon_temp2:
*	dw	0x0000
*z80mon_temp3:
*	dw	0x0000
*z80mon_temp4:
*	dw	0x0000
*z80mon_temp5:
*	dw	0x0000
*z80mon_temp6:
*	dw	0x0000
*z80mon_temp7:
*	dw	0x0000
*z80mon_temp8:
*	dw	0x0000

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
	move	%sr, %d0				/* Disable external interrupts */
	or.l	#0x0700, %d0
	move	%d0, %sr

	/* Search for an init module */
	lea	bootrom_start, %a4			/* Set search start */
	lea	bootrom_start + bootrom_size, %a5	/* Set search end */
	lea	startup_cold_search_rtn, %a6		/* Set return address */

startup_cold_search:
	bra.s	module_find				/* Find next module */

startup_cold_search_rtn:
	beq.s	startup_cold_end			/* If don't find a module, just continue boot */
	cmp.b	#249, %d0				/* Check if it's an init module */
	bne.s	startup_cold_search_next		/* It's not an init module, keep searching */
	adda.l	#0x40, %a4				/* Add offset to module code */
	lea	startup_cold_end, %a6			/* Save return address in A7 */
	jmp	(%a4)					/* Execute init module */
startup_cold_search_next:
	adda.l	#0x100, %a4				/* Increment pointer */
	cmp.l	%a5, %a4				/* Check if we've reached the end */
	blt.s	startup_cold_search			/* We're not at the end so keep searching */
startup_cold_end:

# startup_warm
#################################
# This code is executed once basic system is initialised
# It's assumed that a stack is available at this point
startup_warm:
	mov.l	#stack_ptr, %sp				/* Reset stack pointer */

	lea	bootrom_start, %a4			/* Set search start */
	mov.b	#253, %d1				/* Search for startup application */
	bsr.s	module_search
	cmp.b	#0, %d0
	beq.s	startup_warm_end			/* No module found, so finish */

	adda.l	#0x40, %a4				/* Add offset to module code */
	jsr	(%a4)					/* Execute startup module */
startup_warm_end:

# startup_final
#################################
# Final section of startup, executes menu system
startup_final:
	jsr	print_newline
	jsr	print_newline

	lea	str_logon1, %a0				/* Print greeting */
	jsr	print_cstr
	*lea	str_logon2				/* Print documentation notes */
	*jsr	print_cstr

	jsr	module_list_commands			/* Print included commands */

	jmp	menu_main				/* Enter main menu */

# get_version
#################################
#  Returns the version number of the monitor
#	Out:	D0 = Minor version number [16] | Major version number [16]
get_version:
	mov.l	#m68kmon_version, %d0
	rts

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
	mov.l	%a4, %d0				/* Make sure we are on a boundary */
	andi.l	#0xffffff00, %d0
	mov.l	%d0, %a4
module_find_loop:
	mov.l	%a4, %a1				/* Copy address */
	cmp.b	#0xA5, (%a1)+				/* Check first byte */
	bne.s	module_find_next			/* If this doesn't match, check next range */
	cmp.b	#0xE5, (%a1)+				/* Check second byte */
	bne.s	module_find_next			/* If this doesn't match, check next range */
	cmp.b	#0xE0, (%a1)+				/* Check third byte */
	bne.s	module_find_next			/* If this doesn't match, check next range */
	cmp.b	#0xA5, (%a1)+				/* Check fourth byte */
	bne.s	module_find_next			/* If this doesn't match, check next range */
	mov.b	(%a1), %d0				/* Get module type */
	jmp	(%a6)					/* Resume execution from where we left off */
module_find_next:
	adda.l	#0x100, %a4				/* Increment address pointer */
	cmp.l	%a5, %a4				/* Check if we've reached the end */
	blt.s	module_find_loop
module_find_end:
	eor.l	%d0, %d0				/* Clear D0 to indicate no module */
	jmp	(%a6)					/* Resume execution from where we left off */

# module_search
#################################
# Finds the next header in the external memory.
#	In:	A4 = Address to start searching from
#		D1 = Module type to search for
#	Out:	A4 = location of next module
#		D0 = Found module type (cleared on failure)
module_search:
	lea	module_search_mem_end, %a5		/* Set search end */
	lea	module_search_reenter, %a6		/* Set return address */
module_search_next:
	cmp.l	%a5, %a4				/* Check if we've reached the end */
	bge.s	module_search_failed			/* We've reached the end, so we've failed to find a module */
	jmp	module_find
module_search_reenter:
	beq.s	module_search_failed			/* No module, exit */
	cmp.b	%d1, %d0				/* Check if module type we're looking for */
	beq.s	module_search_end			/* If they're the same, return */
	adda.l	#0x100, %a4				/* Increment address pointer */
	bra.s	module_search_next			/* Continue search */
module_search_failed:
	eor.l	%d0, %d0				/* Clear D0 as we've failed */
module_search_end:
	and.b	%d0, %d0				/* Set status bits appropriately */
	rts

# String routines
###########################################################################
# char_2_upper
#################################
#  Converts ASCII letter to uppercase if necessary
#	In:	D0 = ASCII character
#	Out:	D0 = ASCII character
char_2_upper:
	cmp.b	#97, %d0				/* 'a' */
	blt.s	char_2_upper_end
	cmp.b	#123, %d0				/* 'z' + 1 */
	bgt.s	char_2_upper_end
	add.b	#224, %d0				/* Addition wraps */
char_2_upper_end:
	rts

# char_2_hex
#################################
#  Converts (if possible) a character (A-Z/0-9) to hex value
#	In:	A = ASCII character
#	Out:	A = Hex value
#		Carry set if value valid, not otherwise
*char_2_hex:
*	add	a, 208
*	jr	nc, char_2_hex_not
*	add	a, 246
*	jr	c, char_2_hex_maybe
*	add	a, 10
*	scf						; Set Carry flag
*	ret
*char_2_hex_maybe:
*	add	a, 249
*	jr	nc, char_2_hex_not
*	add	a, 250
*	jr	c, char_2_hex_not
*	add	a, 16
*	scf						; Set Carry flag
*	ret
*char_2_hex_not:
*	scf						; Clear Carry flag
*	ccf
*	ret

# string_length
#################################
#  Calculates the length of the string
#	In:	A0 = Pointer to string
#	Out:	D0 = String length
string_length:
	eor.l	%d0, %d0				/* Clear D0 */
string_length_loop:
	mov.b	(%a0)+, %d1				/* Copy, check whether character is zero, increment pointer */
	beq.s	string_length_end			/* EOL */
	addi.w	#1, %d0					/* Increment character count */
	btst	#7, %d1					/* See if we have a multi-part string */
	bne.s	string_length_end
	bra.s	string_length_loop			/* Loop */
string_length_end:
	rts

# Print routines
###########################################################################
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
# print_spaces_n
#################################
#  Print n spaces
#	In:	D1 = Number of spaces
print_spaces_n:
	subi.w	#1, %d1
print_spaces_n_loop:
	jsr	print_space
	dbf	%d1, print_spaces_n_loop
print_spaces_n_exit:
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
	mov.l	%d3, %d2				/* Copy long */
	swap	%d2					/* Swap top and bottom 16 bits */
	jsr	print_hex16				/* Print top 2 bytes */
	mov.l	%d3, %d2				/* Copy long */
# print_hex16
#################################
#  Print 16 bit number as hex
#	In:	D2 = 16-bit Integer
print_hex16:
	mov.w	%d2, %d1				/* Copy word */
	lsr	#8, %d1					/* Shift top byte down */
	jsr	print_hex8				/* Print top byte */
	mov.w	%d2, %d1				/* Copy word */
# print_hex8
#################################
#  Print 8 bit number as hex
# 	In:	D1 = 8-bit Integer
print_hex8:
	mov.b	%d1, %d0				/* Copy byte */
	lsr	#4, %d0					/* Shift top nibble down */
	jsr	print_hex_digit
	mov.b	%d1, %d0				/* Copy byte */
print_hex_digit:
	and.b	#0xf, %d0				/* Extract bottom nibble */
	add.b	#'0', %d0				/* Add base character */
	cmp.b	#'9', %d0				/* Check whether it's in range 0-9 */
	ble.s	print_hex_digit_out
	add.b	#7, %d0
print_hex_digit_out:
	jsr	console_out				/* Print character */
	rts

*; # print_dec8u
*; #################################
*;  Print 8 bit number as unsigned decimal
*;	In:	A = 8-bit Integer
*print_dec8u:
*	ld	ix, z80mon_temp1			; Used to store state
*	res	0, (ix+0)				; Flag used to suppress leading zeros
*
*	push	bc
*	jr	print_dec8
*; # print_dec8s
*; #################################
*;  Print 8 bit number as signed decimal
*;	In:	A = 8-bit Integer
*print_dec8s:
*	ld	ix, z80mon_temp1			; Used to store state
*	res	0, (ix+0)				; Flag used to suppress leading zeros
*
*	push	bc
*	bit	7, a					; Test whether negative
*	jr	z, print_dec8				; If it's not just continue
*	push	af					; Save value
*	ld	a, '-'
*	call	monlib_console_out
*	pop	af					; Restore value
*	cpl						; Remove sign, and create positive value - 1
*	inc	a
*print_dec8:
*	ld	b, 100
*	call	print_dec8_digit
*	ld	b, 10
*	call	print_dec8_digit
*	set	0, (ix+0)				; Force print of last digit
*	ld	b, 1
*	call	print_dec8_digit
*	pop	bc
*	ret
*; # print_dec8_digit
*; #################################
*;	In:	A = Value
*;		B = Divider
*;		IX = Pointer to flag for zero print
*print_dec8_digit:
*	ld	c, '0' - 1				; Setup digit character
*print_dec8_digit_loop:
*	inc	c					; Increment character digit
*	sub	a, b					; Subtract divisor
*	jr	nc, print_dec8_digit_loop		; Loop if still greater than zero
*	add	b					; Undo last operation
*	push	af					; Save current result
*	ld	a, c
*	cp	'0'					; Check if printing zero
*	jr	nz, print_dec8_digit_print		; If not, just print
*	bit	0, (ix+0)				; Check if should print zero anyway
*	jr	nz, print_dec8_digit_print		; Print it
*	jr	print_dec8_digit_skipped		; Otherwise, skip print
*print_dec8_digit_print:
*	call	monlib_console_out
*	set	0, (ix+0)				; Set zero digit print
*print_dec8_digit_skipped:
*	pop	af					; Restore result
*	ret

*; # print_dec16u
*; #################################
*;  Print a 16 bit number as unsigned decimal
*;	In: 	BC = Integer value
*print_dec16u:
*	ld	ix, z80mon_temp1			; Used to store state
*	xor	a
*	ld	(ix+0), a 				; Clear flag used to suppress leading zeros

*print_dec16u_d5:
*	ld	de, 0x2710
*	call	math_divide_16b				; Divid by 10000
*	ld	a, c
*	ld	b, h					; Copy HL -> BC
*	ld	c, l
*	and	a					; Check there's something to print
*	jr	z, print_dec16u_d4
*	call	print_hex_digit
*	set	4, (ix+0)				; Digit has been printed

*print_dec16u_d4:
*	ld	de, 0x03e8
*	call	math_divide_16b				; Divid by 1000
*	ld	a, c
*	ld	b, h					; Copy HL -> BC
*	ld	c, l
*	bit	4, (ix+0)				; Test whether a digit has been printed already
*	jr	nz, print_dec16u_d4_out
*	and	a					; Check there's something to print
*	jr	z, print_dec16u_d3
*print_dec16u_d4_out:
*	call	print_hex_digit
*	set	3, (ix+0)				; Digit has been printed
*
*print_dec16u_d3:
*	ld	de, 0x0064
*	call	math_divide_16b				; Divid by 100
*	ld	a, c
*	ld	b, h					; Copy HL -> BC
*	ld	c, l
*	bit	3, (ix+0)				; Test whether a digit has been printed already
*	jr	nz, print_dec16u_d3_out
*	and	a					; Check there's something to print
*	jr	z, print_dec16u_d2
*print_dec16u_d3_out:
*	call	print_hex_digit
*	set	2, (ix+0)				; Digit has been printed
*
*print_dec16u_d2:
*	ld	de, 0x000a
*	call	math_divide_16b				; Divid by 10
*	ld	a, c
*	ld	b, h					; Copy HL -> BC
*	ld	c, l
*	bit	2, (ix+0)				; Test whether a digit has been printed already
*	jr	nz, print_dec16u_d2_out
*	and	a					; Check there's something to print
*	jr	z, print_dec16u_d1
*print_dec16u_d2_out:
*	call	print_hex_digit
*	set	1, (ix+0)				; Digit has been printed
*
*print_dec16u_d1:
*	ld	a, c					; Remainder
*	call	print_hex_digit
*
*	ret
*
# print_version
#################################
#  Prints the version number
#	In:	D1 = Version number
print_version:
	lea	str_version, %a0
	jsr	print_str_simple
	rts

*	ld	a, 'v'
*	call	monlib_console_out
*	ld	a, d
*	call	print_dec8u
*	ld	a, '.'
*	call	monlib_console_out
*	ld	a, e
*	call	print_dec8u
*	ret

# print_str_simple
#################################
#  Prints a null terminated string.
#  Retains the ability ability to print consecutive strings.
#	In:	A0 = Pointer to string
print_str_simple:
	mov.b	(%a0)+, %d0				/* Get next character and increment pointer */
	beq.b	print_str_simple_end			/* Check whether NULL character */
	jsr	console_out				/* Print character */
	bra.s	print_str_simple			/* Loop */
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
	eor.l	%d1, %d1				/* Clear D1 for dbeq later */
	mov.b	(%a0)+, %d1				/* Get character count and increment pointer */
	beq.b	print_str_repeat_end			/* Check for EOL */
	subi.w	#1, %d1					/* -1 for dbf */
	mov.b	(%a0)+, %d0				/* Get character and increment pointer */
print_str_repeat_loop:
	jsr	console_out				/* Print character */
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
	mov.b	(%a0), %d0				/* Get next character */
	beq.s	print_str_end				/* Check for end of string */
	and.b	#0x7f, %d0				/* Strip stop bit */
	jsr	console_out				/* Print character */
	btst	#7, (%a0)+				/* Test msb, increment pointer */
	bne.s	print_str_end				/* If msb set, stop */
	bra.s	print_str				/* Loop */
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
	eor.l	%d2, %d2				/* Clear D1, used for state information */
	bset	#1, %d2					/* If unset, prints a space in front of compressed word */
	bset	#5, %d2					/* If unset, prints word with capitalised first letter */
print_cstr_next:
	mov.b	(%a0)+, %d0				/* Get next character, increment pointer */
	beq.s	print_cstr_end				/* Check whether NULL character */
	btst	#7, %d0					/* Test msb */
	beq.s	print_cstr_check_13
	mov.b	%d0, %d1
	jsr	dictionary_print_word			/* It's a dictionary word, so print it */
	bra.s	print_cstr_next
print_cstr_check_13:
	cmp.b	#13, %d0				/* Check for control code */
	bne.s	print_cstr_check_14
	jsr	print_newline
	bset	#1, %d2					/* No automatic space */
	bra.s	print_cstr_next				/* Loop for next character */
print_cstr_check_14:
	cmp.b	#14, %d0				/* Check for control code */
	bne.s	print_cstr_check_31
	jsr	print_newline
	bra.s	print_cstr_end				/* Finished */
print_cstr_check_31:
	cmp.b	#31, %d0				/* Check for control code */
	bne.s	print_cstr_char
	bclr	#5, %d2					/* Capitalise the next word */
	bra.s	print_cstr_next				/* Loop for next character */
print_cstr_char:
	bclr	#1, %d2					/* Ensure a space is printed in front of the next compressed word */
	jsr	console_out				/* Print character */
	bra.s	print_cstr_next				/* Loop to next character */
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
	andi.b	#0x7f, %d1				/* Clear msb of word index */
	bclr.l	#1, %d2					/* Check whether to print space */
	bne.s	dictionary_print_word_search_setup
	jsr	print_space				/* Print space */
dictionary_print_word_search_setup:
	lea	common_words, %a1
	bset.l	#7, %d2					/* Make sure we get the bottom nibble to start */
	and.b	%d1, %d1				/* Update flags */
dictionary_print_word_search_loop:
	beq.s	dictionary_print_word_selected_loop	/* Found the word, so print it */
dictionary_print_word_nibble_loop:
	jsr	dictionary_get_next_nibble		/* Scan through nibbles */
	bne.s	dictionary_print_word_nibble_loop	/* Searching for the end of a word (D0 = 0) */
	subq.b	#1, %d1					/* Decrement word index */
	bra.s	dictionary_print_word_search_loop	/* Search again */
dictionary_print_word_selected_loop:
	jsr	dictionary_get_next_nibble		/* Get next compressed character*/
	beq.s	dictionary_print_word_end		/* End of the word, so finish */
	cmp.b	#0x0f, %d0				/* Check whether to select alternate letters */
	beq.s	dictionary_print_word_select_unloved
dictionary_print_word_select_loved:
	lea	dictionary_loved_letters, %a2		/* Get pointer to loved letters */
	bra.s	dictionary_print_word_get_char
dictionary_print_word_select_unloved:
	jsr	dictionary_get_next_nibble		/* Get the next character */
	lea	dictionary_unloved_letters, %a2		/* Get pointer to unloved letters */
dictionary_print_word_get_char:
	subq.b	#1, %d0					/* Decrement index */
	add.l	%d0, %a2				/* Create pointer to letter */
	mov.b	(%a2), %d0				/* Get letter */
	bset	#5, %d2					/* Test whether to capitalise letter, reset flag */
	bne.s	dictionary_print_word_print_char
	jsr	char_2_upper				/* Capitalise letter */
dictionary_print_word_print_char:
	jsr	console_out				/* Print character */
	bra.s	dictionary_print_word_selected_loop	/* Loop through additional characters */
dictionary_print_word_end:
	rts

dictionary_loved_letters:	.ascii	"etarnisolumpdc"	/* 14 most commonly used letters */
dictionary_unloved_letters:	.ascii	"hfwgybxvkqjz"		/* 12 least commonly used letters */

# dictionary_get_next_nibble
#################################
#  Returns the selected nibble from the memory location
#  Prints a selected word from the built-in dictionary
#	In:	A1 = Pointer to memory location
#		D2 = Control byte
#			bit 7 - Used to indicate top/bottom nibble
#	Out:	D0 = Nibble value
dictionary_get_next_nibble:
	eor.l	%d0, %d0				/* Clear D0, will want to do long addition to an address later */
	mov.b	(%a1), %d0
	bchg.l	#7, %d2					/* Test whether we want top or bottom nibble, and toggle */
	bne.s	dictionary_get_next_nibble_bottom	/* We want the top nibble */
dictionary_get_next_nibble_top:
	adda.l	#1, %a1					/* Increment pointer */
	lsr.b	#4, %d0					/* Top nibble shifted to the bottom */
dictionary_get_next_nibble_bottom:
	and.b	#0x0f, %d0
	rts

*; # print_abort
*; #################################
*;  Print the abort string
*print_abort:
*	ld	hl, str_abort
*	jp	print_cstr

*; # print_registers
*; #################################
*;  Print out the contents of all registers (without altering them!)
*command_print_registers:
*	ld	hl, str_tag_regdump
*	call	print_cstr				; Print message
*print_registers:
*	ld	(z80mon_temp1), sp			; Save stack pointer address
*	ex	(sp), hl				; Swap HL <-> return address
*	ld	(z80mon_temp2), hl			; Save return address
*	ex	(sp), hl				; Swap return address <-> HL
*
*	push	af					; Make a copy of all registers
*	push	bc					; Need to restore them later
*	push	de
*	push	hl
*	ex	af, af'					; Swap AF for shadow registers
*	exx						; Swap for shadow registers
*	push	af					; Make a copy of all registers
*	push	bc					; Need to restore them later
*	push	de
*	push	hl
*	push	ix
*	push	iy
*
*	; Now push a copy of everything on to the stack to print (in the order we're going to read)
*	push	iy
*	push	hl
*	push	de
*	push	bc
*	push	af
*	ex	af, af'
*	exx
*	push	ix
*	push	hl
*	push	de
*	push	bc
*	push	af
*
*	; Print registers
*	call	print_newline
*	ld	hl, str_reg_af1
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_bc1
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_de1
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_hl1
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_ix
*	call	print_str
*	pop	hl
*	call	print_hex16
*	call	print_newline
*
*	; Print shadow registers
*	ld	hl, str_reg_af2
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_bc2
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_de2
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_hl2
*	call	print_str
*	pop	hl
*	call	print_hex16
*	ld	hl, str_reg_iy
*	call	print_str
*	pop	hl
*	call	print_hex16
*	call	print_newline
*
*	; Print additional registers
*	ld	hl, str_reg_sra
*	call	print_str
*	ld	hl, (z80mon_temp2)
*	call	print_hex16
*	ld	hl, str_reg_sp
*	call	print_str
*	ld	hl, (z80mon_temp1)
*	call	print_hex16
*	call	print_newline
*
*	pop	iy					; Restore shadow register contents
*	pop	ix
*	pop	hl
*	pop	de
*	pop	bc
*	pop	af
*	exx						; Exchange
*	ex	af, af'
*	pop	hl					; Restore register contents
*	pop	de
*	pop	bc
*	pop	af
*
*	ret

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
	jsr	console_in				/* Get character if there is one */
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

*; # input_hex8_preloaded
*; #################################
*;  Routine to enter up to 2 digit hexadecimal number
*;	In:	B = Preload value
*;	Out:	DE = Hex value
*;		Carry flag set if value valid
*input_hex8_preloaded:
*	ld	a, b					; Get first digit
*	srl	a					; Shift top nible to the bottom
*	srl	a
*	srl	a
*	srl	a
*	push	af					; Push digit onto the stack
*	call	print_hex_digit				; Print digit
*	ld	a, b					; Get second digit
*	and	0x0f					; Remove top nibble
*	push	af					; Push digit onto the stack
*	call	print_hex_digit				; Print digit
*
*	ld	b, 0x02					; Set digit count to max
*	ld	c, 0x02					; Max. enterable digits
*	jr	input_hex_get_char
*; # input_hex8
*; #################################
*;  Routine to enter up to 2 digit hexadecimal number
*;	Out:	DE = Hex value
*;		Carry flag set if value valid
*input_hex8:
*	ld	b, 0x00					; Clear digit count
*	ld	c, 0x02					; Max. enterable digits
*	jr	input_hex_get_char
*; # input_hex16_preloaded
*; #################################
*;  Routine to enter up to 4 digit hexadecimal number
*;	In:	BC = Preload value
*;	Out:	DE = Hex value
*;		Carry flag set if value valid
*input_hex16_preloaded:
*	ld	a, b					; Get first digit
*	srl	a					; Shift top nible to the bottom
*	srl	a
*	srl	a
*	srl	a
*	push	af					; Push digit onto the stack
*	call	print_hex_digit				; Print digit
*	ld	a, b					; Get second digit
*	and	0x0f					; Remove top nibble
*	push	af					; Push digit onto the stack
*	call	print_hex_digit				; Print digit
*
*	ld	a, c					; Get third digit
*	srl	a					; Shift top nible to the bottom
*	srl	a
*	srl	a
*	srl	a
*	push	af					; Push digit onto the stack
*	call	print_hex_digit				; Print digit
*	ld	a, c					; Get fourth digit
*	and	0x0f					; Remove top nibble
*	push	af					; Push digit onto the stack
*	call	print_hex_digit				; Print digit
*
*	ld	b, 0x04					; Set digit count to max
*	ld	c, 0x04					; Max. enterable digits
*	jr	input_hex_get_char
*; # input_hex16
*; #################################
*;  Routine to enter up to 4 digit hexadecimal number
*;	Out:	DE = Hex value
*;		Carry flag set if value valid
*input_hex16:
*	ld	b, 0x00					; Clear digit count
*	ld	c, 0x04					; Max. enterable digits
*; # input_hex_get_char
*; #################################
*;  Base routine to enter hex ASCII digit(s), and convert that to the equivalent hex value.
*;	In:	B = Current digit count
*;		C = Maximum number of digits
*;	Out:	DE = Hex value
*;		Carry flag set if value valid
*input_hex_get_char:
*	call	input_character_filter			; Get character
*	call	char_2_upper
*input_hex_process_char:
*	ld	d, a					; Copy character
*
*	cp	character_code_escape			; Check whether character is escape key
*	jr	z, input_hex_abort
*	cp	character_code_backspace		; Check whether character is backspace key
*	jr	z, input_hex_delete_digit
*	cp	character_code_delete			; Check whether character is delete key
*	jr	z, input_hex_delete_digit
*	cp	character_code_carriage_return		; Check whether character is CR key
*	jr	z, input_hex_complete
*
*	ld	a, b					; Check that number of digits
*	sub	c					; Is less than (n)
*	jr	nc, input_hex_get_char			; Already have (n) digits, so just loop
*	ld	a, d					; Reload character
*	call	char_2_hex				; Convert ASCII to hex
*	jr	nc, input_hex_get_char			; Character not valid hex digit so loop
*	push	af					; Push hex value on to stack
*	inc	b					; Increment digit count
*	ld	a, d					; Reload character
*	call	monlib_console_out			; Output character
*	jr	input_hex_get_char
*input_hex_delete_digit:
*	ld	a, b					; Check if there are digits to delete
*	cp	0x00
*	jr	z, input_hex_get_char			; No existing digits, so just wait for next character
*	ld	a, d					; Reload character
*	call	monlib_console_out			; Update display
*	pop	af					; Pop digit from stack
*	dec	b					; Decrement digit count
*	jr	input_hex_get_char
*input_hex_abort:
*	xor	a					; Clear A
*	cp	b					; Check if there's anything to remove from the stack
*	jr	z, input_hex_abort_end			; Nothing to pop, so finish
*input_hex_abort_loop:
*	pop	af					; Pop digit
*	djnz	input_hex_abort_loop			; Keep looping until all digits removed
*input_hex_abort_end:
*	ld	de, 0x0000				; Zero register
*	scf						; Clear Carry flag
*	ccf
*	ret
*input_hex_complete:
*	ld	de, 0x0000				; Zero register
*	ld	c, b					; Swap number of digits
*	ld	b, 4					; Total number of shifts
*	xor	a					; Clear A
*	cp	c					; Check if there's anything to remove from the stack
*	jr	z, input_hex_complete_end		; Nothing to pop, so finish
*input_hex_complete_loop:
*	; Make some room for a nibble
*	srl	d					; Right shift into Carry MSB (bit 1)
*	rr	e
*	srl	d					; Right shift into Carry MSB (bit 2)
*	rr	e
*	srl	d					; Right shift into Carry MSB (bit 3)
*	rr	e
*	srl	d					; Right shift into Carry MSB (bit 4)
*	rr	e
*
*	xor	a					; Clear A
*	cp	c					; Check whether we have all the digits
*	jr	z, input_hex_complete_check_remainder
*	; Add digit
*	pop	af					; Pop digit
*	sla	a					; Shit digit in to upper nibble
*	sla	a
*	sla	a
*	sla	a
*	or	d					; OR bits from D (LSB) and shifted digit
*	ld	d, a					; Save combined digit
*	dec	c					; Decrement digit count
*input_hex_complete_check_remainder:
*	djnz	input_hex_complete_loop			; Keep looping until all digits moved
*input_hex_complete_end:
*	scf						; Set Carry flag
*	ret

*; # input_addrs_start_end
*; #################################
*;  Routine to get a start and end address
*;	Out:	BC = Start Address
*;		DE = End Address
*input_addrs_start_end:
*	call	print_newlinex2
*	ld	hl, str_start_addr
*	call	print_cstr
*	call	input_hex16				; Get start address
*	jr	c, input_addrs_start_end_next_addr	; If it's valid, get next address
*	pop	af					; Dump return address off stack
*	jp	print_abort				; So when this returns, it returns to the menu
*input_addrs_start_end_next_addr:
*	push	de					; Store start address
*	call	print_newline
*	ld	hl, str_end_addr
*	call	print_cstr
*	call	input_hex16
*	jr	c, input_addrs_start_end_check
*	pop	af					; Dump start address
*	pop	af					; Dump return address off stack
*	jp	print_abort				; So when this returns, it returns to the menu
*input_addrs_start_end_check:
*	pop	bc
*
*	; Check end address is greater (or equal) than start address
*	ld	a, d
*	cp	b
*	jr	c, input_addrs_start_end_invalid	; End MSB is greater than start MSB, so just finish
*	jr	nz, input_addrs_start_end_finish	; D > B, so finish
*	ld	a, e
*	cp	c
*	jr	c, input_addrs_start_end_invalid
*input_addrs_start_end_finish:
*	jp	print_newline
*input_addrs_start_end_invalid:
*	pop	af					; Dump return address off stack
*	call	print_newline
*	ld	hl, str_invalid
*	jp	print_cstr

*; # input_str
*; #################################
*;  Input a string of upto buffer size - 1. Null terminates string.
*;	In:	B = Size of input buffer
*;		DE = Pointer to string buffer
*;	Out:	A = Character count
*;		Carry flag set if value valid
*input_str:
*	dec	b					; Max number of characters is buffer size - 1, to accept null character at end
*	ld	c, 0x00					; Byte count
*input_str_get_char:
*	call    input_character_filter                  ; Get character
*
*	cp	character_code_escape			; Check whether character is escape key
*	jr	z, input_str_abort
*	cp	character_code_backspace		; Check whether character is backspace key
*	jr	z, input_str_delete_char
*	cp	character_code_delete			; Check whether character is delete key
*	jr	z, input_str_delete_char
*	cp	character_code_carriage_return		; Check whether character is CR key
*	jr	z, input_str_complete
*
*	ex	af, af'
*	ld	a, b					; Get buffer size
*	cp	c					; Compare to byte count
*	jr	z, input_str_get_char			; If equal, do nothing
*	ex	af, af'
*	ld	(de), a					; Copy character to string buffer
*	call	monlib_console_out
*	inc	de					; Increment string buffer pointer
*	inc	c					; Increment byte count
*	jr	input_str_get_char
*input_str_delete_char:
*	xor	a
*	cp	c					; Check if there are characters to delete
*	jr	z, input_str_get_char			; No existing characters, so just wait for next character
*	ld	a, character_code_backspace
*	call	monlib_console_out			; Back 1 character
*	ld	a, " "
*	call	monlib_console_out			; Blank character
*	ld	a, character_code_backspace
*	call	monlib_console_out			; Back 1 character
*	dec	de					; Decrement string buffer pointer
*	dec	c					; Decrement byte count
*	jr	input_str_get_char
*input_str_complete:
*	xor	a					; Clear A
*	ld	(de), a					; Write null character to buffer
*	ld	a, c
*	scf						; Set Carry flag
*	ret
*input_str_abort:
*	scf						; Clear Carry flag
*	ccf
*	ret

# Memory routines
###########################################################################
# memory_copy
#################################
*;  Copies a region of memory to another region
*;	In:	BC = Source Address
*;		DE = Destination Address
*;		HL = Num. bytes
*memory_copy:
*	ld	a, (bc)					; Get byte to copy
*	ld	(de), a					; Save byte
*	inc	bc					; Increment pointers
*	inc	de
*	dec	hl					; Decrement byte count
*	ld	a, l
*	or	h					; See if we've reached zero
*	jr	nz, memory_copy				; If there are remaining bytes to copy
*	ret

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

# Main routines
###########################################################################
# module_list_commands
#################################
#  Print a list of additional commands
command_module_list_commands:
	lea	str_tag_listm, %a0
	jsr	print_cstr				/* Print message */
module_list_commands:
	lea	str_prompt9, %a0
	jsr	print_cstr
	mov.w	#21, %d1
	jsr	print_spaces_n
	#lea	str_prompt9b, %a0
	jsr	print_cstr

	lea	module_search_mem_start, %a4		/* Set search start */
	lea	module_search_mem_end, %a5		/* Set search end */
module_list_commands_next:
	lea	module_list_commands_reenter, %a6	/* Set return address */
	bra.w	module_find				/* Find next module */
module_list_commands_reenter:
	beq.w	module_list_commands_exit		/* If not, return */

	jsr	print_spacex2
	mov.l	%a4, %a0				/* Reset offset to command name */
	adda.w	#0x20, %a0
	jsr	print_str				/* Print command name */
	mov.l	%a4, %a0				/* Reset offset to command name */
	adda.w	#0x20, %a0
	jsr	string_length				/* Get command name length */
	mov.w	#31, %d1				/* Need word here */
	sub.b	%d0, %d1				/* Calculate padding */
	jsr	print_spaces_n				/* Print padding */
	mov.l	%a4, %d3				/* Reset offset to module start */
	jsr	print_hex32				/* Print module address */
	mov.w	#4, %d1
	jsr	print_spaces_n				/* Print padding */
	mov.l	%a4, %a0				/* Reset offset to command type */
	adda.w	#0x04, %a0
	mov.b	(%a0), %d0				/* Get command type */

	lea	str_type5, %a0				/* Type unknown */
module_list_commands_type_35:
	cmp.b	#35, %d0				/* Program */
	bne.s	module_list_commands_type_249
	lea	str_type2, %a0
	bra.s	module_list_commands_type_print
module_list_commands_type_249:
	cmp.b	#249, %d0				/* Init */
	bne.s	module_list_commands_type_253
	lea	str_type3, %a0
	bra.s	module_list_commands_type_print
module_list_commands_type_253:
	cmp.b	#253, %d0				/* Startup command */
	bne.s	module_list_commands_type_254
	lea	str_type4, %a0
	bra.s	module_list_commands_type_print
module_list_commands_type_254:
	cmp.b	#254, %d0				/* External command */
	bne.s	module_list_commands_type_print
	lea	str_type1, %a0
module_list_commands_type_print:
	jsr	print_cstr				/* Print type */
	jsr	print_newline

	adda.l	#0x100, %a4				/* Increment address pointer */
	cmp.l	%a5, %a4				/* Check if we've reached the end */
	blt.w	module_list_commands_next		/* Loop */
module_list_commands_exit:
	jmp	print_newline				/* Finish */

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
	jsr	print_cstr				/* Print message */
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
	lea	module_search_mem_start, %a4		/* Set search start */
command_help_external_commands_loop:
	mov.b	#0xfe, %d1				/* Search for external command */
	jsr	module_search
	beq.s	command_help_end			/* No command found so finish */
	jsr	print_spacex2
	mov.l	%a4, %a0				/* Offset to module command character */
	adda.w	#0x5, %a0
	mov.b	(%a0), %d0				/* Get module command character */
	jsr	console_out				/* Print character */
	jsr	print_dash_spaces
	mov.l	%a4, %a0				/* Offset to module command name */
	adda.w	#0x20, %a0
	jsr	print_str				/* Print module name */
	jsr	print_newline
	adda.w	#0x100, %a4				/* Increment module search start address */
	bra.s	command_help_external_commands_loop
command_help_end:
	jmp	print_newline				/* Print newline and return */

*; # command_location_new
*; #################################
*;  Sets the monitor pointer to where default operations are performed
*command_location_new:
*	ld	hl, str_tag_nloc
*	call	print_cstr				; Print message
*
*	ld	hl, str_prompt6				; Print location prompt
*	call	print_cstr
*	call	input_hex16				; Get value
*	jp	nc, print_abort				; If escaped, print abort message
*	ld	(MONITOR_ADDR_CURRENT), de		; Save value
*	jp	print_newline

*; # command_stack_change
*; #################################
*;  Sets the monitor pointer to where default operations are performed
*command_stack_change:
*	ld	hl, str_tag_stack
*	call	print_cstr				; Print message
*
*	ld	hl, str_prompt14			; Print location prompt
*	call	print_cstr
*	call	input_hex16				; Get value
*	jp	nc, print_abort				; If escaped, print abort message
*	ex	de, hl					; Move to the needed register
*	ld	sp, hl					; Set stack pointer
*	rst	16					; Restart monitor

*; # command_jump
*; #################################
*;  Request an address, and jump to the code at that location
*command_jump:
*	ld	hl, str_tag_jump
*	call	print_cstr				; Print message
*
*	ld	hl, str_prompt8
*	call	print_cstr
*	ld	hl, str_prompt4
*	call	print_cstr
*	ld	bc, (MONITOR_ADDR_CURRENT)
*	call	input_hex16_preloaded
*	jr	c, command_jump_prep
*	jp	print_abort
*command_jump_prep:
*	push	de
*	ld	hl, str_runs
*	call	print_cstr
*	pop	hl
*	call	print_hex16
*	call	print_newline
*
*	ld	bc, 0x0010				; Reset address (RST 16: z80Mon final startup)
*	push	bc					; Push reset address on stack
*	push	bc					; In case there's a RET at the end of the code
*command_jump_brkpnt:
*	jp	(hl)					; Execute startup module

*; # command_hexdump
*; #################################
*;  Dump memory at the default location
*command_hexdump:
*	ld	hl, str_tag_hexdump
*	call	print_cstr				; Print message
*
*	ld	hl, (MONITOR_ADDR_CURRENT)
*	ld	de, 0x0600
*
*	call	print_newline
*command_hexdump_line_print:
*	ld	e, 0x08					; Number of bytes per line
*	call	print_spacex2
*	call	print_hex16
*	call	print_colon_space
*command_hexdump_line_print_loop:
*	ld	a, (hl)
*	call	print_hex8
*	inc	hl
*	ld	a, (hl)
*	call	print_hex8
*	inc	hl
*	call	print_spacex2
*	dec	e					; Decrement line byte count
*	jr	nz, command_hexdump_line_print_loop
*	call	print_newline
*	dec	d					; Decrement line count
*	jr	nz, command_hexdump_line_print
*
*	push	hl
*	ld	hl, str_prompt15
*	call	print_cstr
*	pop	hl
*	call	input_character_filter			; Next page or quit
*	cp	character_code_escape			; Check if quit
*	jr	z, command_hexdump_end
*	ld	de, 0x600				; Another 0x600 bytes
*	jr	command_hexdump_line_print
*command_hexdump_end:
*	ret

*; # command_edit
*; #################################
*;  Basic memory editor
*command_edit:
*	ld	hl, str_tag_edit
*	call	print_cstr				; Print message
*
*	ld	hl, str_edit1
*	call	print_cstr
*	ld	hl, (MONITOR_ADDR_CURRENT)		; Get default address
*command_edit_loop:
*	call	print_hex16				; Print address
*	call	print_colon_space
*	ld	b, (hl)					; Load memory contents
*	call	input_hex8_preloaded			; Edit loaded value
*	jr	c, command_edit_save			; Check if Escape was pressed
*	jp	print_abort
*command_edit_save:
*	ld	(hl), e					; Save editted value back to memory
*	call	print_newline
*	inc	hl					; Increment memory pointer
*	ld	(MONITOR_ADDR_CURRENT), hl		; Save memory pointer as default
*	jr	command_edit_loop

*; # command_clear_mem
*; #################################
*;  Clears a region of memory
*command_clear_mem:
*	ld	hl, str_tag_clrmem
*	call	print_cstr				; Print message
*
*	call	input_addrs_start_end
*	ld	(z80mon_temp1), bc			; Save start/end address
*	ld	(z80mon_temp2), de			; They're about to be trashed
*	ld	hl, str_sure
*	call	print_cstr
*	call	input_character_filter			; Get response
*	call	char_2_upper
*	cp	'Y'					; Compare key to 'Y'
*	jr	z, command_clear_mem_do
*	ld	a, 'N'
*	call	monlib_console_out
*	jp	print_newlinex2
*command_clear_mem_do:
*	ld	a, 'Y'
*	call	monlib_console_out
*	call	print_newline
*	ld	bc, (z80mon_temp1)			; Reload start/end addresses
*	ld	de, (z80mon_temp2)
*command_clear_mem_loop:
*	xor	a					; Clear A
*	ld	(bc), a					; Clear current memory address
*	ld	a, b					; Check address MSB
*	cp	d
*	jr	nz, command_clear_mem_inc
*	ld	a, c					; Check address LSB
*	cp	e
*	jr	nz, command_clear_mem_inc
*	ld	hl, str_clrcomp
*	call	print_cstr
*	ret
*command_clear_mem_inc:
*	inc	bc
*	jr	command_clear_mem_loop

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

*; # command_upload
*; #################################
*;  Uploads a selected section of memory
*command_upload:
*	ld	hl, str_tag_upld
*	call	print_cstr				; Print message
*
*	call	input_addrs_start_end
*	push	de					; Save addresses
*	push	bc					; print_cstr trashes everything
*	ld	hl, str_upld1
*	call	print_cstr
*	pop	de					; Restore address
*	ld	h, d					; Copy DE->HL
*	ld	l, e
*	call	print_hex16				; Print start address
*	ld	hl, str_upld2
*	call	print_str				; Use str not cstr as it only trashes HL
*	pop	bc					; Restore address
*	push	bc					; Save addresses again
*	push	de
*	ld	h, b					; Copy BC->HL
*	ld	l, c
*	call	print_hex16				; Print end address
*	call	print_newline
*	ld	hl, str_prompt7
*	call	print_cstr
*	call	monlib_console_in			; Get character
*	cp	character_code_escape			; Check if escape
*	jr	nz, command_upload_send
*	pop	bc					; Pop saved addresses
*	pop	de
*	jp	print_abort
*command_upload_send:
*	call	print_newline
*	pop	de					; Pop saved addresses
*	pop	hl
*	inc	hl
*command_upload_send_calc_remain:
*	push	hl					; Store end address
*	sbc	hl, de
*	ld	a, h					; Check value
*	and	a
*	jr	nz, command_upload_send_line_16
*	ld	a, l					; Check value
*	and	0xf0
*	jr	nz, command_upload_send_line_16
*	ld	b, l
*	pop	hl
*	jr	command_upload_send_line_n
*command_upload_send_line_16:
*	ld	b, 0x10
*	pop	hl
*command_upload_send_line_n:
*	call	command_upload_intel_hex_line
*
*	ld	a, h					; Check remaining
*	cp	d
*	jr	nz, command_upload_send_calc_remain
*	ld	a, l					; Check remaining
*	sub	e
*	jr	z, command_upload_send_eof
*	jr	nc, command_upload_send_calc_remain
*
*command_upload_send_eof:
*	call	command_upload_intel_hex_line_eof
*
*	ret
*
*; Writes a line of data in intel hex format
*; B = Number of bytes to output
*; HL = Pointer to area of memory to read
*command_upload_intel_hex_line:
*	ld	c, 0x00					; Zero C (checksum value)
*
*	ld	a, ':'					; line start character
*	call	monlib_console_out
*
*	ld	a, b					; Get byte count
*	call	print_hex8
*	add	c					; Add to checksum
*	ld	c, a					; Save checksum
*
*	ld	a, d					; Get address high byte
*	call	print_hex8
*	add	c					; Add to checksum
*	ld	c, a					; Save checksum
*
*	ld	a, e					; Get address low byte
*	call	print_hex8
*	add	c					; Add to checksum
*	ld	c, a					; Save checksum
*
*	ld	a, 0x00					; Data type
*	call	print_hex8
*command_upload_intel_hex_line_loop:
*	ld	a, (de)					; Get byte from memory
*	call	print_hex8
*	add	c					; Add to checksum
*	ld	c, a					; Save checksum
*	inc	de
*	djnz	command_upload_intel_hex_line_loop	; Loop remain bytes on line
*command_upload_intel_hex_line_checksum:
*	cpl						; Complement A
*	add	0x01					; Add 1
*	call	print_hex8				; Print checksum
*	call	print_newline
*	ret
*
*; Writes out an Intel hex format EOF
*command_upload_intel_hex_line_eof:
*	; Intel hex file EOF
*	ld	a, ':'					; line start character
*	call	monlib_console_out
*	xor	a					; Clear A
*	call	print_hex8
*	call	print_hex8
*	call	print_hex8
*	inc	a
*	call	print_hex8
*	ld	a, 0xff
*	call	print_hex8
*	call	print_newline
*	ret

*; # command_download_alt
*; #################################
*;  Downloads an Intel format hex file
*;  Used directly from the main menu
*command_download_alt:
*	call	command_download_init
*	jr	command_download_line_start
*; # command_download
*; #################################
*;  Downloads an Intel format hex file
*command_download:
*	ld	hl, str_tag_dnld
*	call	print_cstr				; Print message
*
*	ld	hl, str_dnld1
*	call	print_cstr
*	call	command_download_init
*command_download_char_read:
*	call	monlib_console_in			; Get character
*	cp	character_code_escape			; Check for escape
*	jp	z, command_download_abort
*	cp	':'					; Check for line start
*	jr	z, command_download_line_start
*	call	char_2_hex
*	jr	nc, command_download_char_read
*	ld	e, 0x06					; Unexpected hex digits (while waiting for BOL)
*	call	command_download_stats_inc
*	jr	command_download_char_read
*command_download_line_start:
*	ld	e, 0					; (lines received)
*	call	command_download_stats_inc
*command_download_line_start_no_inc:
*	ld	a, '.'					; Print period to indicate transfer
*	call	monlib_console_out
*	ld	c, 0x0					; C = clear checksum
*	call	command_download_get_hex
*	ld	d, a					; D = Number of bytes on this row
*	ld	a, '.'					; Print period to indicate transfer
*	call	monlib_console_out
*	call	command_download_get_hex
*	ld	h, a					; H = MSB address
*	call	command_download_get_hex
*	ld	l, a					; L = LSB address
*	call	command_download_get_hex		; Record type
*	cp	1
*	jr	z, command_download_finished		; Intel hex format EOF
*	and	a
*	jr	nz, command_download_line_unknown
*command_download_line_data:
*	ld	a, d
*	and	a
*	jr	z, command_download_line_checksum
*	call	command_download_get_hex
*	ld	(hl), a					; Save byte of data to memory
*	ld	e, 1
*	call	command_download_stats_inc		; (bytes received)
*	ld	e, 2
*	call	command_download_stats_inc		; (bytes received)
*	inc	hl					; Increment address pointer
*	dec	d
*	jr	nz, command_download_line_data
*command_download_line_checksum:
*	call	command_download_get_hex
*	ld	a, c					; Get checksum value
*	and	a					; Which should be zero
*	jr	z, command_download_char_read
*command_download_line_checksum_error:
*	ld	e, 4
*	call	command_download_stats_inc		; (incorrect checksums)
*	jr	command_download_char_read
*command_download_line_unknown:
*	ld	a, d					; Check payload size
*	jr	z, command_download_line_checksum
*command_download_line_unknown_data:
*	call	command_download_get_hex
*	dec	d
*	jr	nz, command_download_line_unknown_data
*	jr	command_download_line_checksum
*command_download_finished:
*	ld	a, d
*	jr	z, command_download_finished_summary
*command_download_finished_data:				; There shouldn't be data to consume...
*	call	command_download_get_hex
*	dec	d
*	jr	nz, command_download_finished_data
*command_download_finished_summary:
*	call	command_download_get_hex		; Get checksum
*	ld	a, c
*	and	a					; Check checksum
*	jp	nz, command_download_summary_error_print
*	call	download_delay
*	ld	hl, str_dnld3
*	call	print_cstr				; Download okay
*; Could do with unconditionally accepting any additional characters for a short time
*	jp	command_download_summary
*command_download_abort:
*	call	download_delay
*	ld	hl, str_dnld2
*	call	print_cstr
*	jr	command_download_summary
*
*; Used to zero the monitor temp space which is used
*; to store stats
*command_download_init:
*	ld	b, 0x10					; Number of bytes to clear
*	ld	hl, z80mon_temp				; Address of temp space
*	xor	a					; Clear A
*command_download_init_loop:
*	ld	(hl), a					; Clear memory
*	inc	hl					; Increment pointer
*	djnz	command_download_init_loop
*	ret
*
*; Increment download stat indexed by E
*; 0 = lines received		4 = incorrect checksums
*; 1 = bytes received		5 = unexpected begin of line
*; 2 = bytes written		6 = unexpected hex digits (while waiting for BOL)
*; 3 = bytes unable to write	7 = unexpected non-hex digits (in middle of a line)
*command_download_stats_inc:
*	push	bc					; Save registers
*	push	de
*	push	ix
*	sla	e					; Index x2 (16 bit value)
*	ld	d, 0					; Adding the pair to IX
*	ld	ix, z80mon_temp				; Get base address variable store
*	add	ix, de					; Add index
*	ld	c, (ix+0)				; Get 16 bit value
*	ld	b, (ix+1)
*	inc	bc					; Increment value
*	ld	(ix+0), c
*	ld	(ix+1), b
*	pop	ix					; Restore registers
*	pop	de
*	pop	bc
*	ret
*
*; Gets a hexadecimal value from input, returns it in A. Adds value of A to C (checksum value)
*; Does not print character. Does not handle CR, or backspace.
*command_download_get_hex:
*	call	monlib_console_in			; Get character
*	call	char_2_upper
*	cp	character_code_escape			; Check whether character is escape key
*	jr	nz, command_download_get_hex_check_char
*command_download_get_hex_escape:
*	pop	af					; Pop return address
*	jr	command_download_abort			; Jump to abort message
*command_download_get_hex_check_char:
*	cp	':'
*	jr	nz, command_download_get_hex_process_char
*command_download_get_hex_check_char_unexpected_bol:
*	ld	e, 5					; (unexpected begin of line)
*	call	command_download_stats_inc
*	pop	af					; Pop return address
*	jp	command_download_line_start_no_inc
*command_download_get_hex_process_char:
*	call	char_2_hex				; Convert ASCII to hex
*	jr	c, command_download_get_hex_process_value
*	ld	e, 7					; (unexpected non-hex digits)
*	call	command_download_stats_inc
*	jr	command_download_get_hex
*command_download_get_hex_process_value:
*	sla	a					; Shift lower nibble to upper nibble
*	sla	a
*	sla	a
*	sla	a
*	ld	b, a					; Save value
*command_download_get_hex_next_char:
*	call	monlib_console_in			; Get character
*	call	char_2_upper
*	cp	character_code_escape			; Check whether character is escape key
*	jr	z, command_download_get_hex_escape
*command_download_get_hex_check_next_char:
*	cp	':'
*	jr	z, command_download_get_hex_check_char_unexpected_bol
*command_download_get_hex_process_next_char:
*	call	char_2_hex				; Convert ASCII to hex
*	jr	c, command_download_get_hex_process_next_value
*	ld	e, 7					; (unexpected non-hex digits)
*	call	command_download_stats_inc
*	jr	command_download_get_hex_next_char
*command_download_get_hex_process_next_value:
*	or	b					; Combine values
*	push	af					; Save value
*	add	a, c					; Add to checksum
*	ld	c, a					; Store new checksum value
*	pop	af					; Restore value
*	ret
*
*; Prints download stats
*command_download_summary:
*	ld	hl, str_dnld4
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp)
*	call	print_dec16u
*	ld	hl, str_dnld5
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+2)
*	call	print_dec16u
*	ld	hl, str_dnld6a
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+4)
*	call	print_dec16u
*	ld	hl, str_dnld6b
*	call	print_cstr
*
*	; Check for errors
*	ld	b, 5
*	ld	hl, 0x0000
*	ld	ix, z80mon_temp+6
*command_download_summary_error_chk:
*	ld	e, (ix+0)
*	inc	ix
*	ld	d, (ix+0)
*	inc	ix
*	add	hl, de
*	djnz	command_download_summary_error_chk
*
*	xor	a					; Clear A
*	or	h
*	or	l
*	and	a					; Check if any errors
*	jr	nz, command_download_summary_error_print
*	ld	hl, str_dnld13				; No errors
*	call	print_cstr
*	jr	command_download_summary_finish
*
*command_download_summary_error_print:
*	ld	hl, str_dnld7
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+6)
*	call	print_dec16u
*	ld	hl, str_dnld8
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+8)
*	call	print_dec16u
*	ld	hl, str_dnld9
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+10)
*	call	print_dec16u
*	ld	hl, str_dnld10
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+12)
*	call	print_dec16u
*	ld	hl, str_dnld11
*	call	print_cstr
*
*	call	print_space
*	ld	bc, (z80mon_temp+14)
*	call	print_dec16u
*	ld	hl, str_dnld12
*	call	print_cstr
*
*command_download_summary_finish:
*	jp	print_newline

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
	lea	str_prompt1, %a0			/* First part of the prompt */
	jsr	print_cstr
	mov.l	MONITOR_ADDR_CURRENT, %d3		/* Get current address */
	jsr	print_hex32
	lea	str_prompt2, %a0			/* Second part of the prompt */
	jsr	print_str

	jsr	input_character_filter			/* Get character input */

#	cmp.b	#':', %d0				/* Check for ':' from pushing a HEX file from a terminal */
#	bne.s	menu_main_push_address
#	jsr	command_download_alt
#	bra.s	menu_main

menu_main_push_address:
	jsr	char_2_upper				/* Convert to uppercase to simplify matching */
	lea	menu_main, %a0
	mov.l	%a0, -(%sp)				/* Push menu_main address to stack to make returning easier */
	mov.b	%d0, %d4				/* Save command character */

#menu_main_external_commands:
#	mov.l	module_search_mem_end, %a4		/* Set search start */
#menu_main_external_commands_loop:
#	mov.b	#0xfe, %d1				/* Search for external command */
#	jsr	module_search
#	beq.s	menu_main_builtin_commands		/* No command found so procede with builtin commands */
#	mov.l	%a4, %a0				/* Make offset to command character */
#	adda.w	#0x5, %a0
#	cmp.b	(%a0), %d4				/* Are they the same character? */
#	beq.s	menu_main_external_commands_exec	/* Execute external command */
#	adda.w	#0x100, %a4				/* Increment module search start address */
#	bra.s	menu_main_external_commands_loop	/* Loop */
#menu_main_external_commands_exec:
#	jsr	print_space
#	mov.l	%a4, %a0				/* Make offset to command name */
#	adda.w	#0x20, %a0
#	jsr	print_str				/* Print module command name */
#	jsr	print_newline
#	mov.l	%a4, %a0				/* Make offset to module code */
#	adda.w	#0x40, %a0
#	jmp	(%a0)					/* Execute external command */

menu_main_builtin_commands:
	cmp.b	#command_key_help, %d4			/* Check if help key */
	beq.w	command_help				/* Run command */

*	cp	command_key_help			; Check if help key
*	jp	z, command_help				; Run command
*	cp	command_key_listm			; Check if list modules key
*	jp	z, command_module_list_commands		; Run command
*	cp	command_key_regdump			; Check if dump registers key
*	jp	z, command_print_registers		; Run command
*	cp	command_key_new_locat			; Check if new location key
*	jp	z, command_location_new			; Run command
*	cp	command_key_stack			; Check if changestack location key
*	jp	z, command_stack_change			; Run command
*	cp	command_key_jump			; Check if jump key
*	jp	z, command_jump				; Run command
*	cp	command_key_hexdump			; Check if hexdump key
*	jp	z, command_hexdump			; Run command
*	cp	command_key_edit			; Check if edit key
*	jp	z, command_edit				; Run command
*	cp	command_key_clrmem			; Check if clear memory key
*	jp	z, command_clear_mem			; Run command
*	;cp	command_key_run				; Check if run key
*	;jp	z, command_run				; Run command
*	cp	command_key_in				; Check if in key
*	jp	z, command_port_in			; Run command
*	cp	command_key_out				; Check if out key
*	jp	z, command_port_out			; Run command
*	cp	command_key_upload			; Check if upload key
*	jp	z, command_upload			; Run command
*	cp	command_key_download			; Check if download key
*	jp	z, command_download			; Run command

menu_main_end:
	jmp	print_newline				/* This will return to menu_main */

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

*; # List of strings
*; #################################
*; Strings used to dump register contents
*str_reg_af1:		db	" AF =&",0
*str_reg_af2:		db	" AF'=&",0
*str_reg_bc1:		db	" BC =&",0
*str_reg_bc2:		db	" BC'=&",0
*str_reg_de1:		db	" DE =&",0
*str_reg_de2:		db	" DE'=&",0
*str_reg_hl1:		db	" HL =&",0
*str_reg_hl2:		db	" HL'=&",0
*str_reg_ix:		db	" IX =&",0
*str_reg_iy:		db	" IY =&",0
*str_reg_sp:		db	" SP =&",0
*str_reg_sra:		db	" SRA =&",0

#str_logon1:		db	"Welcome",128," z80Mon v0.1",13,14			/* Welcome string (OLD) */
#str_logon2:	 	db	32,32,"See",148,"2.DOC,",148,"2.EQU",164
#			db	148,"2.HDR",180,213,141,".",14				/* Documentation string */
str_logon1:		.ascii	"Welcome"						/* Welcome string */
			dc.b	128
			.ascii	" m68kMon"
			dc.b	14
#str_prompt1:		db	148,"2 Loc:",0						/* Paulmon2 Loc: (OLD) */
str_prompt1:		.asciz	"m68kMon:"						/* m68kMon: */
str_prompt2:		dc.b	' ','>',160						/*  > abort run which program(	(must follow after prompt1) */
#str_prompt3:		db	134,202,130,'(',0					/* run which program( */
#str_prompt4:		db	"),",149,140,128,200,": ",0				/* ), or esc to quit: (OLD) */
str_prompt4:		dc.b	',',149,31,140,':',' ',0				/* , or Escape: */
#str_prompt5:		db	31,151,130,195,"s",199,166,131,","
#			db	186," JUMP",128,134,161,"r",130,13,14			/* No program headers found in memory, use JUMP to run your program */
str_prompt6:		dc.b	13,13,31,135,131,129,':',' ',0				/* \n\nNew memory location: */
str_prompt7:		dc.b	31,228,251,' ','k','e','y',':',' ',0			/* Press any key: */
#str_prompt8:		db	13,13,31,136,128,131,129," (",0				/* \n\nJump to memory location ( (OLD) */
str_prompt8:		dc.b	13,13,31,136,128,131,129,0				/* \n\nJump to memory location */
#str_prompt9:		db	13,13,31,130,31,253,0					; \n\nProgram Name (OLD)
str_prompt9:		dc.b	13,31,130,31,253,0					/* \nProgram Name */
str_prompt9b:		dc.b	31,129,32,32,32,32,32,31,201,14				/* Location      Type	 (must follow prompt9) */
str_prompt10:		dc.b	')',' ',31,135,31,178,':',' ',0				/* ) New Value: */
str_prompt11:		dc.b	31,189,':',' ',0					/* Port: */
str_prompt12:		dc.b	31,178,':',' ',0					/* Value: */
str_prompt14:		dc.b	13,13,31,135,227,129,':',' ',0				/* \n\nNew stack location: */
str_prompt15:		dc.b	31,228,251						/* Press any key for next page, or esc to quit */
			.ascii	" key"
			dc.b	180,212
			.ascii	" page,"
			dc.b	149,140,128,200,14

str_type1:		dc.b	31,154,158,0						/* External command */
str_type2:		dc.b	31,130,0						/* Program */
str_type3:		.asciz	"Init"							/* Init */
str_type4:		dc.b	31,143,31,226,31,170,0					/* Start Up Code */
str_type5:		.asciz	"???"							/* ??? */

str_tag_help2:		dc.b	31,215,0						/* Help */
str_tag_help1: 		dc.b	31,142,215,209,0					/* This help list (these 11 _cmd string must be in order) */
str_tag_listm:  	dc.b	31,209,130,'s',0					/* List Programs */
#str_tag_run:  		dc.b	31,134,130,0						/* Run Program */
str_tag_dnld: 		dc.b	31,138,0						/* Download */
str_tag_upld: 		dc.b	31,147,0						/* Upload */
str_tag_nloc: 		dc.b	31,135,129,0						/* New Location */
str_tag_jump: 		dc.b	31,136,128,131,129,0					/* Jump to memory location */
#str_tag_dump: 		dc.b	31,132,219,154,131,0					/* Hex dump external memory (OLD) */
str_tag_hexdump: 	dc.b	31,132,219,131,0					/* Hex dump memory */
str_tag_in:		.ascii	"Read"							/* Read in port */
			dc.b	166,189,0
str_tag_out:		dc.b	31,225,128,189,0					/* Write to port */
str_tag_regdump: 	dc.b	31,219,31,196,'s',0					/* Dump Registers */
#str_tag_edit: 		dc.b	31,156,154,146,0					/* Editing external ram (OLD) */
str_tag_edit: 		dc.b	31,216,31,146,0						/* Edit Ram */
str_tag_clrmem: 	dc.b	31,237,131,0						/* Clear memory */
str_tag_stack:		dc.b	31,240,227,129,0					/* Change stack location */

str_help1:		dc.b	13,13							/* \n\nStandard Commands */
			.ascii	"Standard"
			dc.b	31,158,'s',14
str_help2:		dc.b	31,218,31,244,'e','d',31,158,'s',14			/* User Installed Commands */
#str_abort:		dc.b	' ',31,158,31,160,'!',13,14				/*  Command Abort!\n\n */
str_abort:		dc.b	' ',31,158,31,160,'!',14				/*  Command Abort!\n */
#str_runs:		dc.b	13,134,'n','i','n','g',130,':',13,14			/* \nRunning program:\n\n */
str_runs:		dc.b	13,134,'n','i','n','g',130,' ','@',0			/* \nRunning program @ */

#str_edit1: 		dc.b	13,13,31,156,154,146,',',140,128,200,14			/* \n\nEditing external ram, esc to quit\n (OLD) */
str_edit1: 		dc.b	13,13,31,156,31,146,',',31,140,128,200,14		/* \n\nEditing Ram, Esc to quit\n */
str_edit2: 		dc.b	' ',' ',31,156,193,',',142,129,247,13,14		/*   Editing complete, this location unchanged\n\n */

str_start_addr:		dc.b	31,143,31,254,':',' ',0
str_end_addr:		dc.b	31,248,31,254,':',' ',0
str_sure:		dc.b	31,185,161,' ','s','u','r','e','?',0			/* Are you sure? */
str_clrcomp:		dc.b	31,131,237,193,14					/* Memory clear complete\n */

str_invalid:		.ascii	"Invalid selection"
			dc.b	14

str_upld1: 		dc.b	13,13							/* \n\nSending Intel hex file from */
			.ascii	"Sending"
			dc.b	31,152,132,137,172,32,32,0
str_upld2:		dc.b	' ','t','o',' ',0					/* to */
#str_upld2: 		dc.b	' ',128,32,32,0						/*  to */

str_dnld1: 		dc.b	13,13,31,159						/* \n\nBegin ascii transfer of Intel hex file */
			.ascii	" ascii"
			dc.b	249,150,31,152,132,137
			dc.b	',',149,140,128,160,13,14				/* , or esc to abort \n\n */
str_dnld2: 		dc.b	13,31,138,160,'e','d',13,14				/* \nDownload aborted\n\n */
str_dnld3: 		dc.b	13,31,138,193,'d',13,14					/* \nDownload completed\n\n */
str_dnld4: 		dc.b	13							/* \nSummary:\n */
			.ascii	"Summary:"
			dc.b	14
str_dnld5: 		dc.b	' ',198,'s',145,'d',14					/*  lines received\n */
str_dnld6a:		dc.b	' ',139,145,'d',14					/*  bytes received\n */
str_dnld6b:		dc.b	' ',139							/*  bytes written\n */
			.ascii	" written"
			dc.b	14
str_dnld7: 		dc.b	31,155,':',14						/* Errors:\n */
str_dnld8: 		dc.b	' ',139							/*  bytes unable to write\n */
			.ascii	" unable"
			dc.b	128
			.ascii	" write"
			dc.b	14
str_dnld9: 		dc.b	32,32,'b','a','d',245,'s',14				/* bad checksums\n */
str_dnld10:		dc.b	' ',133,159,150,198,14					/*  unexpected begin of line\n */
str_dnld11:		dc.b	' ',133,132,157,14					/*  unexpected hex digits\n */
str_dnld12:		dc.b	' ',133,' ','n','o','n',132,157,14			/*  unexpected non hex digits\n */
str_dnld13:		dc.b	31,151,155						/* No errors detected\n\n */
			.ascii	" detected"
			dc.b	13,14

str_ny:			.asciz	" (N/y): "
str_version:		.asciz	"Version: "

*; ##########################################################################################################################################
*; ##########################################################################################################################################
*; ##########################################################################################################################################
*;; These three parameters tell PAULMON2 where the user's memory is
*;; installed.  "bmem" and "emem" define the space that will be searched
*;; for program headers, user installed commands, start-up programs, etc.
*;; "bmem" and "emem" should be use so they exclude memory areas where
*;; perphreal devices may be mapped, as reading memory from an io chip
*;; may reconfigure it unexpectedly.  If flash rom is used, "bmem" and "emem"
*;; should also include the space where the flash rom is mapped.
*;
*;.equ	pgm, 0x8000+base			; default location for the user program
*;.equ	bmem, 0x1000+base			; where is the beginning of memory
*;.equ	emem, 0xFFFF+base			; end of the memory
*;
*;;---------------------------------------------------------;
*;;							  ;
*;;	                OsyterLib			  ;
*;;							  ;
*;;---------------------------------------------------------;
*;.equ	oysterlib_base,  0x1000+base
*;.equ	oysterlib_locat, 0x80+oysterlib_base
*;.equ	oysterlib_cout, 0x00+oysterlib_locat
*;.equ	oysterlib_newline, 0x03+oysterlib_locat
*;.equ	oysterlib_cin, 0x06+oysterlib_locat
*;.flag   use_oysterlib, 0x20.0
*;
*;; To set the baud rate, use this formula or set to 0 for auto detection
*;; baud_const = 256 - (crystal / (12 * 16 * baud))
*;
*;.equ	baud_const, 0				; automatic baud rate detection
*;;.equ	baud_const, 255				; 57600 baud w/ 11.0592 MHz
*;;.equ	baud_const, 253				; 19200 baud w/ 11.0592 MHz
*;;.equ	baud_const, 252				; 19200 baud w/ 14.7456 MHz
*;;.equ	baud_const, 243				; 4808 baud w/ 12 MHz
*;
*;.equ	line_delay, 6				; num of char times to pause during uploads
*;
*;; About download speed: when writing to ram, PAULMON2 can accept data
*;; at the maximum baud rate (baud_const=255 or 57600 baud w/ 11.0592 MHz).
*;; Most terminal emulation programs introduce intentional delays when
*;; sending ascii data, which you would want to turn off for downloading
*;; larger programs into ram.  For Flash ROM, the maximum speed is set by
*;; the time it takes to program each location... 9600 baud seems to work
*;; nicely for the AMD 28F256 chip.  The "character pacing" delay in a
*;; terminal emulation program should be sufficient to download to flash
*;; rom and any baud rate.  Some flash rom chips can write very quickly,
*;; allowing high speed baud rates, but other chips can not.  You milage
*;; will vary...
*;
*;; These symbols configure paulmon2's internal memory usage.
*;; It is usually not a good idea to change these unless you
*;; know that you really have to.
*;
*;.equ	psw_init, 0				; value for psw (which reg bank to use)
*;.equ	dnld_parm, 0x10				; block of 16 bytes for download
*;;.equ	stack, 0x30				; location of the stack
*;.equ	stack, 0x70				; location of the stack
*;.equ	baud_save, 0x68				; save baud for warm boot, 4 bytes
*;
*;;---------------------------------------------------------;
*;;							  ;
*;;		     Interrupt Vectors			  ;
*;;  (and little bits of code crammed in the empty spaces)  ;
*;;							  ;
*;;---------------------------------------------------------;
*;
*;.org	base
*;
*;r6r7todptr:
*;	mov	dpl, r6
*;	mov	dph, r7
*;	ret
*;
*;dptrtor6r7:
*;	mov	r6, dpl
*;	mov	r7, dph
*;	ret
*;
*;
*;esc:  ;checks to see if <ESC> is waiting on serial port
*;      ;C=clear if no <ESC>, C=set if <ESC> pressed
*;      ;buffer is flushed
*;	push	acc
*;	clr	c
*;	jnb	ri,esc2
*;	mov	a,sbuf
*;	cjne	a,#27,esc1
*;	setb	c
*;esc1:	clr	ri
*;esc2:	pop	acc
*;	ret
*;
*;
*;line_dly: ;a brief delay between line while uploading, so the
*;	;receiving host can be slow (i.e. most windows software)
*;	mov	a, r0
*;	push	acc
*;	mov	r0, #line_delay*2
*;line_d2:mov	a, th0		;get baud rate const
*;line_d3:inc	a
*;	nop
*;	nop
*;	jnz	line_d3
*;	djnz	r0, line_d2
*;	pop	acc
*;	mov	r0, a
*;	ret
*;
*;;---------------------------------------------------------;
*;
*;pop_it: pop	acc
*;	pop	acc
*;abort_it:
*;	acall	newline
*;abort2: mov	dptr, #abort
*;	ajmp	pcstr_h
