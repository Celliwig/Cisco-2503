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

# Include definitions
.include "cisco-2500/cisco-2500.h"
.include "include/stddefs.h"

.org	monitor_start

*; # Architecture Jump Table
*; ###########################################################################
*; RST 0 / Processor cold start
*orgmem	mem_base
*	di						; Disable maskable interrupts
*	jp	startup_cold				; Cold startup
*
*; RST 8
*orgmem	mem_base+0x08
*	jp	startup_warm				; Main startup routine (stack must be available)
*
*; RST 16
*orgmem	mem_base+0x10
*	jp	startup_final				; Startup completed
*
*; RST 24
*orgmem	mem_base+0x18
*	ds	8
*
*; RST 32
*orgmem	mem_base+0x20
*	ds	8
*
*; RST 40
*orgmem	mem_base+0x28
*	ds	8
*
*; RST 48
*orgmem	mem_base+0x30
*	ds	8
*
*; RST 56 / Mode 1 maskable interrupt
*orgmem	mem_base+0x38
*z80_interrupt_handler:
*	reti						; RETI added to ignore maskable interrupt
*	reti						; Additional library may overwrite
*	reti						; With actual interrupt handler
*	reti
*
*; NMI
*orgmem	mem_base+0x66
*z80_nm_interrupt_handler:
*	retn						; RETN added to ignore non-maskable interrupt
*	retn						; Additional library may overwrite
*	retn						; With actual interrupt handler
*	retn
*
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
*z80mon_default_addr:
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
*
*; # Monitor
*; ###########################################################################
*orgmem	mon_base
*
*; # get_version
*; #################################
*;  Returns the version number of the monitor
*;	Out:	D = Major version number
*;		E = Minor version number
*get_version:
*	ld	d, z80mon_version_major
*	ld	e, z80mon_version_minor
*	ret
*
*; # Math routines
*; ###########################################################################
*; # math_divide_16b
*; #################################
*;  Divides 16 bit number by another 16 bit number
*;  From: http://map.grauw.nl/sources/external/z80bits.html
*;	In:	BC = Dividend
*;		DE = Divisor
*;		HL = 0
*;	Out:	BC = Quotient
*;		HL = Remainder
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
*
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
*
*; # Print routines
*; ###########################################################################
*; Note: will not alter any registers other than AF.
*; Shadow registers fair game.
*;
*; # print_spacex2
*; #################################
*;  Print double space
*print_spacex2:
*	call	print_space
*; # print_space
*; #################################
*;  Print a space
*print_space:
*	ld	a, ' '
*	jp	monlib_console_out
*; # print_spaces_n
*; #################################
*;  Print n spaces
*;	In:	B = Number of spaces
*print_spaces_n:
*	call	print_space
*	dec	b
*	jr	nz, print_spaces_n
*print_spaces_n_exit:
*	ret
*
*; # print_dash
*; #################################
*;  Print '-'
*print_dash:
*	ld	a, '-'
*	jp	monlib_console_out
*; # print_dash_spaces
*; #################################
*;  Print ' - '
*print_dash_spaces:
*	call	print_space
*	call	print_dash
*	jr	print_space
*
*; # print_colon_space
*; #################################
*;  Print ': '
*print_colon_space:
*	ld	a, ":"
*	call	monlib_console_out
*	jp	print_space
*
*;cout_sp:acall	cout
*;	ajmp	space
*;	nop
*;
*
*; # print_newlinex2
*; #################################
*;  Print 2 new lines
*print_newlinex2:					; Print two newlines
*	call	print_newline
*; # print_newline
*; #################################
*;  Print a new line
*print_newline:
*	ld	a, character_code_carriage_return
*	call	monlib_console_out
*	ld	a, character_code_linefeed
*	call	monlib_console_out
*	ret
*
*; # print_hex16
*; #################################
*;  Print 16 bit number as hex
*;	In:	HL = 16-bit Integer
*print_hex16:
*	ld	a, h
*	call	print_hex8
*	ld	a, l
*; # print_hex8
*; #################################
*;  Print 8 bit number as hex
*; 	In:	A = 8-bit Integer
*print_hex8:
*	push	af
*	rrca
*	rrca
*	rrca
*	rrca
*	call	print_hex_digit
*	pop	af
*print_hex_digit:
*	push	af
*	and	0x0F
*	add	a, 0x90
*	daa
*	adc	a, 0x40
*	daa
*	call	monlib_console_out
*	pop	af
*	ret
*
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
*
*; # print_dec16u
*; #################################
*;  Print a 16 bit number as unsigned decimal
*;	In: 	BC = Integer value
*print_dec16u:
*	ld	ix, z80mon_temp1			; Used to store state
*	xor	a
*	ld	(ix+0), a 				; Clear flag used to suppress leading zeros
*
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
*
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
*; # print_version
*; #################################
*;  Prints the version number
*;	In:	D = Major version number
*;		E = Minor version number
*print_version:
*	ld	a, 'v'
*	call	monlib_console_out
*	ld	a, d
*	call	print_dec8u
*	ld	a, '.'
*	call	monlib_console_out
*	ld	a, e
*	call	print_dec8u
*	ret
*
*; # print_str_simple
*; #################################
*;  Prints a null terminated string.
*;  Retains the ability ability to print consecutive strings.
*;	In:	HL = Pointer to string
*print_str_simple:
*	ld	a, (hl)					; Get next character
*	inc	hl					; Increment pointer
*	and	a					; Test if value is zero
*	jr	z, print_str_simple_end			; Check whether NULL character
*	call	monlib_console_out			; Print character
*	jr	print_str_simple			; Loop
*print_str_simple_end:
*	ret
*
*print_str_simple_space:
*	call	print_str_simple
*	jp	print_space
*
*print_str_simple_newline:
*	call	print_str_simple
*	jp	print_newline
*
*; # print_str_repeat
*; #################################
*;  Prints a string of repeated character.
*;  Format:	n(0) = Num. char. to print, n(1) = Character to print
*;		Repeats until null character
*;	In:	HL = Pointer to string
*print_str_repeat:
*	ld	a, (hl)
*	inc	hl					; Increment pointer
*	and	a
*	jr	z, print_str_repeat_end
*	ld	b, a					; Character count
*	ld	c, (hl)					; Get character to print
*	inc	hl					; Increment pointer
*print_str_repeat_loop:
*	ld	a, c
*	call	monlib_console_out			; Print character
*	djnz	print_str_repeat_loop
*	jr	print_str_repeat
*print_str_repeat_end:
*	ret
*
*; # print_str
*; #################################
*;  Prints a null terminated string.
*;  Retains the ability ability to print consecutive strings.
*;	In:	HL = Pointer to string
*print_str:
*	ld	a, (hl)					; Get next character
*	inc	hl					; Increment pointer
*	and	a					; Test if value is zero
*	jr	z, print_str_end			; Check whether NULL character
*	push	af					; Save value
*	and	0x7f					; Strip msb
*	call	monlib_console_out			; Print character
*	pop	af					; Restore value
*	bit	7, a					; Test value msb
*	jr	nz, print_str_end			; If msb set, stop
*	jr	print_str				; Loop
*print_str_end:
*	ret
*
*; # print_cstr
*; #################################
*;  Prints compressed strings. A dictionary of 128 words is stored in 4
*;  bit packed binary format. When it finds a byte in a string with the
*;  high bit set, it prints the word from the dictionary. A few bytes
*;  have special functions and everything else prints as if it were an
*;  ordinary string.
*;
*;  Special codes for pcstr:
*;	0 = end of string
*;	13 = CR/LF
*;	14 = CR/LF and end of string
*;	31 = next word code should be capitalized
*;
*;	In:	HL = Pointer to compressed string
*print_cstr:
*	ld	b, 0					; B is used to store state information
*	set	1, b					; If unset, prints a space in front of compressed word
*	set	5, b					; If unset, prints word with capitalised first letter
*print_cstr_next:
*	ld	a, (hl)					; Get next character
*	inc	hl					; Increment pointer
*	and	a					; Test if value is zero
*	jr	z, print_cstr_end			; Check whether NULL character
*	bit	7, a					; Test msb
*	jr	z, print_cstr_check_13
*	call	dictionary_print_word			; It's a dictionary word, so print it
*	jr	print_cstr_next
*print_cstr_check_13:
*	cp	13					; Check for control code
*	jr	nz, print_cstr_check_14
*	call	print_newline
*	set	1, b					; Print a space in front of compressed word
*	jr	print_cstr_next				; Loop for next character
*print_cstr_check_14:
*	cp	14					; Check for control code
*	jr	nz, print_cstr_check_31
*	call	print_newline
*	jr	print_cstr_end
*print_cstr_check_31:
*	cp	31					; Check for control code
*	jr	nz, print_cstr_char
*	res	5, b					; Reset B(5) so as to capitalise next word
*	jr	print_cstr_next
*print_cstr_char:
*	res	1, b					; Print a space in front of next compressed word.
*	call	monlib_console_out			; Print character
*	jr	print_cstr_next
*print_cstr_end:
*	ret
*
*; # dictionary_print_word
*; #################################
*;  Prints a selected word from the built-in dictionary
*;	In:	A = Selected word index
*;		B = Control byte
*;			bit 1 - 0 = Print space / 1 = Don't print space
*;			bit 5 - 0 = Capitalise first letter / 1 = Don't capitalise first letter
*;			bit 7 - Used to indicate top/bottom nibble
*dictionary_print_word:
*	and	0x7F					; Clear msb on A
*	ld	c, a					; Save word index
*	bit	1, b					; If unset, print a space
*	jr	nz, dictionary_print_word_search_setup
*	call	print_space				; Print a space
*dictionary_print_word_search_setup:
*	res	1, b					; Be sure to print a space next time
*	ld	ix, common_words			; Get pointer to dictionary
*	res	7, b					; Make sure we get the bottom nibble to start
*	ld	a, c					; Reload word index
*	and	c					; Update flags
*dictionary_print_word_search_loop:
*	jr	z, dictionary_print_word_selected_loop	; Found the word, so print it
*dictionary_print_word_nibble_loop:
*	call	dictionary_get_next_nibble		; Scan through nibbles
*	jr	nz, dictionary_print_word_nibble_loop	; Searching for the end of a word (A = 0)
*	dec	c					; Found the end of the word, so decrement word index
*	jr	dictionary_print_word_search_loop	; Search again
*dictionary_print_word_selected_loop:
*	call	dictionary_get_next_nibble		; Get next compressed character
*	jr	z, dictionary_print_word_end		; End of the word, so finish
*	cp	0x0f					; Check whether to select alternate letters
*	jr	z, dictionary_print_word_select_unloved
*dictionary_print_word_select_loved:
*	ld	iy, dictionary_loved_letters		; Get pointer to loved letters
*	jr	dictionary_print_word_get_char
*dictionary_print_word_select_unloved:
*	call	dictionary_get_next_nibble		; Get the next character
*	ld	iy, dictionary_unloved_letters		; Get pointer to unloved letters
*dictionary_print_word_get_char:
*	dec	a					; Decrement index
*	ld	d, 0
*	ld	e, a					; Create offset
*	add	iy, de					; Add offset to pointer
*	ld	a, (iy+0)				; Get letter
*	bit	5, b					; Test whether to capitalise letter
*	jr	nz, dictionary_print_word_print_char
*	call	char_2_upper				; Capitalise letter
*	set	5, b					; Disable capitalisation for the rest of the word
*dictionary_print_word_print_char:
*	push	ix					; Save IX
*	call	monlib_console_out			; Print character
*	pop	ix					; Restore IX
*	jr	dictionary_print_word_selected_loop	; Loop through additional characters
*dictionary_print_word_end:
*	ret
*
*dictionary_loved_letters:	db	"etarnisolumpdc"	; 14 most commonly used letters
*dictionary_unloved_letters:	db	"hfwgybxvkqjz"		; 12 least commonly used letters
*
*; # dictionary_get_next_nibble
*; #################################
*;  Returns the selected nibble from the memory location
*;  Prints a selected word from the built-in dictionary
*;	In:	IX = Pointer to memory location
*;		B = Control byte
*;			bit 7 - Used to indicate top/bottom nibble
*dictionary_get_next_nibble:
*	ld	a, (IX+0)
*	bit	7, b					; Test whether we want top or bottom nibble
*	jr	nz, dictionary_get_next_nibble_top	; We want the top nibble
*dictionary_get_next_nibble_bottom:
*	and	0x0f
*	set	7, b					; Select top nibble on the next read
*	ret
*dictionary_get_next_nibble_top:
*	inc	ix					; Increment pointer
*	srl	a
*	srl	a
*	srl	a
*	srl	a					; Top nibble shifted to the bottom
*	and	0x0f
*	res	7, b					; Select bottom nibble on the next read
*	ret
*
*; # print_abort
*; #################################
*;  Print the abort string
*print_abort:
*	ld	hl, str_abort
*	jp	print_cstr
*
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
*
*; # String routines
*; ###########################################################################
*; # char_2_upper
*; #################################
*;  Converts ASCII letter to uppercase if necessary
*;	In:	A = ASCII character
*;	Out:	A = ASCII character
*char_2_upper:
*	cp	97					; 'a'
*	jr	c, char_2_upper_end
*	cp	123					; 'z' + 1
*	jr	nc, char_2_upper_end
*	add	224
*char_2_upper_end:
*	ret
*
*; # char_2_hex
*; #################################
*;  Converts (if possible) a character (A-Z/0-9) to hex value
*;	In:	A = ASCII character
*;	Out:	A = Hex value
*;		Carry set if value valid, not otherwise
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
*
*; # string_length
*; #################################
*;  Calculates the length of the string
*;	In:	HL = Pointer to string
*;	Out:	BC = String length
*string_length:
*	push	hl
*	ld	bc, 0					; Clear count
*string_length_loop:
*	ld	a, (hl)					; Get byte to test
*	and	a					; Set flags
*	jr	z, string_length_end			; Have we reached the end
*	inc	bc					; Increment count
*	bit	7, a					; See if we have a multi-part string
*	jr	nz, string_length_end
*	inc	hl					; Increment pointer
*	jr	string_length_loop
*string_length_end:
*	pop	hl
*	ret
*
*; # Input routines
*; ###########################################################################
*; # input_character_filter
*; #################################
*;  Routine replaces character sequences for up/down/left/right & PageUp/PageDown
*;  with values noted below.
*;	Return Value	Key		Escape Sequence
*;	11 (^K)		Up		1B 5B 41
*;	10 (^J)		Down		1B 5B 42
*;	21 (^U)		Right		1B 5B 43
*;	8 (^H)		Left		1B 5B 44
*;	25 (^Y)		PageUp		1B 5B 35 7E
*;	26 (^Z)		PageDown	1B 5B 36 7E
*;
*;	Out:	A = ASCII character code
*;	Carry flag set is character value
*input_character_filter:
*	call	monlib_console_in			; Get character if there is one
*;	jnc	input_character_filter_end		; No character available, so finish
*;
*;	cp	character_code_escape			; Compare character to Escape character
*;	jr	nz, input_character_filter_end		; It's not, so finish
*
*	; Need serial port!!!
*	; So just pass through
*
*input_character_filter_end:
*	ret
*
*
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
*
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
*
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
*
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
*
*; # Memory routines
*; ###########################################################################
*; # memory_copy
*; #################################
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
*
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
*
*; # Module routines
*; ###########################################################################
*; # module_find
*; #################################
*;  Finds the next header in the external memory. (Has to be able to work without stack)
*;  	In:	HL = point to start of search (only MSB used)
*;		DE = point to end of search
*;		IX = Return address
*;  	Out:	BC = location of next module
*;		A = module type, or unset if no module found
*;		Carry flag is set on success, Carry flag is unset on failure
*module_find:
*	ld	l, 0x0					; Make sure we are on a boundary
*	ld	b, h					; Copy HL -> BC
*	ld	c, l
*
*	ld	a, 0xA5					; First identity byte
*	cp	(hl)					; Compare memory
*	jr	nz, module_find_next			; Not equal, so check next range
*	inc	hl					; Increment pointer
*	ld	a, 0xE5					; Second identity byte
*	cp	(hl)					; Compare memory
*	jr	nz, module_find_next			; Not equal, so check next range
*	inc	hl					; Increment pointer
*	ld	a, 0xE0					; Third identity byte
*	cp	(hl)					; Compare memory
*	jr	nz, module_find_next			; Not equal, so check next range
*	inc	hl					; Increment pointer
*	ld	a, 0xA5					; Fourth identity byte
*	cp	(hl)					; Compare memory
*	jr	nz, module_find_next			; Not equal, so check next range
*	inc	hl					; Increment pointer
*	ld	a, (hl)					; Get module type
*	scf						; Set Carry flag
*	jp	(ix)					; Resume execution from where we left off
*module_find_next:
*	ld	a, d					; Get end address MSB
*	cp	h					; Compare to current address
*	jr	z, module_find_end			; Were at the end so finish
*	inc	h					; Increment MSB of pointer
*	jr	module_find				; Repeat search
*module_find_end:
*	xor	a					; Clear A to indicate no module
*	scf						; Clear Carry flag
*	ccf
*	jp	(ix)					; Resume execution from where we left off
*
*; # module_search
*; #################################
*;  Finds the next header in the external memory.
*;	In:	A = Module type to search for
*;		HL = Address to start searching from
*;	Out:	HL = location of next module
*;		Carry flag is set on success, Carry flag is unset on failure
*module_search:
*	ld	de, mem_srch_end			; Set search end
*	inc	d					; Increment search end address (so as to search up to and including)
*	ld	ix, module_search_reenter		; Set return address
*	ex	af, af'					; Swap out module type
*module_search_next:
*	ld	a, d					; Copy end search pointer MSB for compare
*	cp	h					; Check whether we are at the end of the search space
*	jr	z, module_search_failed			; We've reached the end, so we've failed to find a module
*	jr	module_find
*module_search_reenter:
*	jr	nc, module_search_failed		; No module, exit
*	ld	h, b					; Copy BC -> HL
*	ld	l, c
*	ld	b, a					; Copy module type
*	ex	af, af'					; Swap in module type
*	cp	b					; Check module type
*	jr	z, module_search_end			; If they're the same, return
*	ex	af, af'					; Swap out module type
*	inc	h					; Increment pointer MSB
*	jr	module_search_next			; Continue search
*module_search_end:
*	scf						; Set Carry flag
*	ret
*module_search_failed:
*	scf
*	ccf						; Clear Carry flag
*	ret
*
*; # module_list_commands
*; #################################
*;  Print a list of additional commands
*command_module_list_commands:
*	ld	hl, str_tag_listm
*	call	print_cstr				; Print message
*module_list_commands:
*	ld	hl, str_prompt9
*	call	print_cstr
*	ld	b, 21
*	call	print_spaces_n
*	;ld	hl, str_prompt9b
*	call	print_cstr
*
*	ld	hl, mem_srch_start			; Set search start
*	ld	de, mem_srch_end			; Set search end
*module_list_commands_next:
*	ld	ix, module_list_commands_reenter	; Set return address
*	jr	module_find
*module_list_commands_reenter:
*	cp	0x00					; Check if we have a module
*	jr	z, module_list_commands_exit		; If not, return
*	ld	h, b					; Copy BC -> HL
*	;ld	l, c					; Don't actually need C as it's overwritten
*	push	de					; Save search end
*	push	bc					; Save module pointer (Restored as HL)
*
*	call	print_spacex2
*	ld	l, 0x20					; Offset to command name
*	call	print_str				; Print command name
*	ld	l, 0x20					; Reset offset
*	call	string_length				; Get command name length
*	ld	l, 0x0					; Reset offset
*	ld	a, 33					; Calculate padding
*	sub	c
*	ld	b, a
*	call	print_spaces_n				; Print padding
*	call	print_hex16				; Print command address
*	ld	b, 0x06
*	call	print_spaces_n				; Print padding
*	ld	l, 0x04					; Offset to command type
*	ld	a, (hl)					; Get command type
*
*	ld	hl, str_type5				; Type unknown
*module_list_commands_type_35:
*	cp	35					; Program
*	jr	nz, module_list_commands_type_249
*	ld	hl, str_type2
*	jr	module_list_commands_type_print
*module_list_commands_type_249:
*	cp	249					; Init
*	jr	nz, module_list_commands_type_253
*	ld	hl, str_type3
*	jr	module_list_commands_type_print
*module_list_commands_type_253:
*	cp	253					; Startup command
*	jr	nz, module_list_commands_type_254
*	ld	hl, str_type4
*	jr	module_list_commands_type_print
*module_list_commands_type_254:
*	cp	254					; External command
*	jr	nz, module_list_commands_type_print
*	ld	hl, str_type1
*module_list_commands_type_print:
*	call	print_cstr				; Print type
*	call	print_newline
*
*	pop	hl					; Restore module pointer
*	pop	de					; Restore search end
*	ld	a, d					; Copy end search pointer MSB for compare
*	cp	h					; Check whether we are at the end of the search space
*	jr	z, module_list_commands_exit		; We've reached the end, so no more modules
*	inc	h					; Increment pointer MSB
*	jr	module_list_commands_next		; Continue search
*module_list_commands_exit:
*	jp	print_newline				; Finish
*
*; # Main routines
*; ###########################################################################
*
*; # command_help_line_print
*; #################################
*;  Prints a line of help text
*;	In:	B = Command key
*;		HL = Command help string
*command_help_line_print:
*	call	print_spacex2
*	ld	a, b
*	call	monlib_console_out
*	call	print_dash_spaces
*	call	print_cstr
*	jp	print_newline
*
*; # command_help
*; #################################
*;  Prints help text
*command_help:
*	ld	hl, str_tag_help2
*	call	print_cstr				; Print message
*command_help_internal_commands:
*	ld	hl, str_help1
*	call	print_cstr
*
*	ld	b, command_key_help
*	ld	hl, str_tag_help1
*	call	command_help_line_print
*	ld	b, command_key_listm
*	;ld	hl, str_tag_listm
*	call	command_help_line_print
*	;ld	b, command_key_run
*	;;ld	hl, str_tag_run
*	;call	command_help_line_print
*	ld	b, command_key_download
*	;ld	hl, str_tag_dnld
*	call	command_help_line_print
*	ld	b, command_key_upload
*	;ld	hl, str_tag_upld
*	call	command_help_line_print
*	ld	b, command_key_new_locat
*	;ld	hl, str_tag_nloc
*	call	command_help_line_print
*	ld	b, command_key_jump
*	;ld	hl, str_tag_jump
*	call	command_help_line_print
*	ld	b, command_key_hexdump
*	;ld	hl, str_tag_hexdump
*	call	command_help_line_print
*	ld	b, command_key_in
*	;ld	hl, str_tag_in
*	call	command_help_line_print
*	ld	b, command_key_out
*	;ld	hl, str_tag_out
*	call	command_help_line_print
*	ld	b, command_key_regdump
*	;ld	hl, str_tag_regdump
*	call	command_help_line_print
*	ld	b, command_key_edit
*	;ld	hl, str_tag_edit
*	call	command_help_line_print
*	ld	b, command_key_clrmem
*	;ld	hl, str_tag_clrm
*	call	command_help_line_print
*	ld	b, command_key_stack
*	;ld	hl, str_tag_stack
*	call	command_help_line_print
*
*command_help_external_commands:
*	ld	hl, str_help2
*	call	print_cstr
*	ld	hl, mem_srch_start			; Set search start
*command_help_external_commands_loop:
*	ld	a, 0xfe					; Search for external command
*	call	module_search
*	jr	nc, command_help_end			; No command found so finish
*	call	print_spacex2
*	ld	l, 0x05					; Offset to module command character
*	ld	a, (hl)					; Get module command character
*	call	monlib_console_out			; Print character
*	call	print_dash_spaces
*	ld	l, 0x20					; Offset to module name
*	call	print_str				; Print module name
*	call	print_newline
*	inc	h					; Increment module search start address
*	jr	command_help_external_commands_loop	; Loop around again
*
*command_help_end:
*	jp	print_newline				; Print newline and return
*
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
*	ld	(z80mon_default_addr), de		; Save value
*	jp	print_newline
*
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
*
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
*	ld	bc, (z80mon_default_addr)
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
*
*
*; # command_hexdump
*; #################################
*;  Dump memory at the default location
*command_hexdump:
*	ld	hl, str_tag_hexdump
*	call	print_cstr				; Print message
*
*	ld	hl, (z80mon_default_addr)
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
*
*; # command_edit
*; #################################
*;  Basic memory editor
*command_edit:
*	ld	hl, str_tag_edit
*	call	print_cstr				; Print message
*
*	ld	hl, str_edit1
*	call	print_cstr
*	ld	hl, (z80mon_default_addr)		; Get default address
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
*	ld	(z80mon_default_addr), hl		; Save memory pointer as default
*	jr	command_edit_loop
*
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
*
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
*
*; # command_port_in
*; #################################
*;  Displays the contents of a particular port
*command_port_in:
*	ld	hl, str_tag_in
*	call	print_cstr				; Print message
*
*	call	print_newlinex2
*	ld	hl, str_prompt11
*	call	print_cstr
*	call	input_hex8				; Get port address
*	jr	c, command_port_in_cont			; Check if escape pressed
*	jp	print_abort
*command_port_in_cont:
*	ld	c, e
*	in	a, (c)					; Read port
*	push	af					; Save value
*	call	print_newline
*	ld	hl, str_prompt12
*	call	print_cstr
*	pop	af
*	call	print_hex8				; Print value
*	call	print_newline
*	ret
*
*; # command_port_out
*; #################################
*;  Writes data to a particular port
*command_port_out:
*	ld	hl, str_tag_out
*	call	print_cstr				; Print message
*
*	call	print_newlinex2
*	ld	hl, str_prompt11
*	call	print_cstr
*	call	input_hex8				; Get port address
*	jr	c, command_port_out_cont1		; Check if escape pressed
*	jp	print_abort
*command_port_out_cont1:
*	push	de					; Save port address
*	call	print_newline
*	ld	hl, str_prompt12
*	call	print_cstr
*	call	input_hex8
*	jr	c, command_port_out_cont2		; Check if escape pressed
*	jp	print_abort
*command_port_out_cont2:
*	pop	bc					; Restore port address
*	out	(c), e					; Write value to port
*	call	print_newline
*	ret
*
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
*
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
*
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
*
*; # menu_main
*; #################################
*;  Implements interactive menu
*menu_main:
*	; First we print out the prompt, which isn't as simple
*	; as it may seem, since external code can add to the
*	; prompt, so we've got to find and execute all of 'em.
*	ld	hl, str_prompt1				; First part of the prompt
*	call	print_cstr
*	ld	hl, (z80mon_default_addr)		; Get current address
*	call	print_hex16
*	ld	hl, str_prompt2				; Second part of the prompt
*	call	print_str
*
*	call	input_character_filter			; Get character input
*
*	cp	':'					; Check for ':' from pushing a HEX file from a terminal
*	jr	nz, menu_main_push_address
*	call	command_download_alt
*	jr	menu_main
*
*menu_main_push_address:
*	call	char_2_upper				; Convert to uppercase to simplify matching
*	ld	bc, menu_main
*	push	bc					; Push menu_main address to stack to make returning easier
*	push	af					; Save command character
*
*menu_main_external_commands:
*	ld	hl, mem_srch_start			; Set search start
*menu_main_external_commands_loop:
*	ld	a, 0xfe					; Search for startup application
*	call	module_search
*	jr	nc, menu_main_builtin_commands		; No command found so procede with builtin commands
*	pop	bc					; Restore command character
*	ld	l, 0x05					; Offset to module command character
*	ld	a, (hl)					; Get module command character
*	cp	b					; Are they the same character?
*	jr	z, menu_main_external_commands_exec	; Execute external command
*	push	bc					; Store command character again
*	inc	h					; Increment module search start address
*	jr	menu_main_external_commands_loop	; Loop around again
*menu_main_external_commands_exec:
*	call	print_space
*	ld	l, 0x20					; Offset to module command name
*	call	print_str				; Print module command name
*	call	print_newline
*	ld	l, 0x40					; Offset to module code
*	xor	a
*	jp	(hl)
*
*menu_main_builtin_commands:
*	pop	af					; Restore command character
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
*
*menu_main_end:
*	jp	print_newline				; This will return to menu_main
*
*; # Startup routines
*; ###########################################################################
*; # startup_cold
*; #################################
*; This is the first section of code executed on startup before any
*; hardware is initialised.
*startup_cold:
*	; Basic initialisation
*	di						; Disable interrupts
*	im	1					; Set interrupt mode 1
*	ld	sp, 0x0					; Zero stack pointer
*
*	; Search for an init module
*	ld	hl, mem_srch_start			; Set search start
*	ld	de, mem_srch_end			; Set search end
*	ld	ix, startup_cold_search_rtn		; Set return address
*startup_cold_search:
*	jp	module_find
*startup_cold_search_rtn:
*	cp	0x00					; Check if we have a module
*	jr	z, startup_cold_end			; If don't find a module, just continue boot
*	ld	h, b					; Copy BC -> HL
*	ld	l, c
*	cp	249					; Check if it's an init module
*	jr	nz, startup_cold_search_next		; It's not an init module, keep searching
*	ld	sp, hl					; Load module base addess in to stack pointer (we can't load IX directly)
*	ld	ix, 0x40				; Load offset in to IX
*	add	ix, sp					; Create pointer to init module
*	jp	(ix)					; Execute init module
*startup_cold_search_next:
*	inc	h					; Increment pointer MSB
*	ld	a, d					; Copy end search pointer MSB for compare
*	cp	h					; Check whether we are at the end of the search space
*	jr	nz, startup_cold_search			; We're not at the end so keep searching
*
*startup_cold_end:
*	rst	8					; We've done basic system init so warm boot rest
*
*; # startup_warm
*; #################################
*; This code is executed once basic system is initialised
*; It's assumed that a stack is available at this point
*startup_warm:
*	ld	a, 253					; Search for startup application
*	ld	hl, mem_srch_start			; Set search start
*	call	module_search
*	jr	nc, startup_warm_end			; No module found, so finish
*
*	ld	de, 0x40				; Load offset to module code
*	add	hl, de					; Create pointer to init module
*	jp	(hl)					; Execute startup module
*
*startup_warm_end:
*	rst	16
*
*; # startup_final
*; #################################
*;  Final section of startup, executes menu system
*startup_final:
*	call	print_newline
*	ld	hl, str_logon1				; Print greeting
*	call	print_cstr
*	;ld	hl, str_logon2				; Print documentation notes
*	;call	print_cstr
*
*	call	module_list_commands			; Print included commands
*
*	jp	menu_main				; Enter main menu
*
*; # Fixed data structures
*; ###########################################################################
*
*; # Dictionary of common words
*; #################################
*;  This is the dictionary of 128 words used by print_cstr.
*; 0 - to		32 - abort		64 - internal		 96 - support
*; 1 - location		33 - you		65 - complete		 97 - write
*; 2 - program		34 - the		66 - an			 98 - up
*; 3 - memory		35 - is			67 - header		 99 - stack
*; 4 - hex		36 - and		68 - register		100 - press
*; 5 - unexpected	37 - interrupt		69 - must		101 - see
*; 6 - run		38 - in			70 - line		102 - reset
*; 7 - new		39 - be			71 - found		103 - pointer
*; 8 - jump		40 - with		72 - quit		104 - fixed
*; 9 - file		41 - as			73 - type		105 - detection
*;10 - download		42 - code		74 - which		106 - may
*;11 - bytes		43 - will		75 - erase		107 - has
*;12 - esc		44 - from		76 - step		108 - assemble
*;13 - information	45 - that		77 - provide		109 - clear
*;14 - this		46 - at			78 - so			110 - configure
*;15 - start		47 - used		79 - single		111 - data
*;16 - rom		48 - if			80 - should		112 - change
*;17 - receive		49 - by			81 - list		113 - allow
*;18 - ram		50 - value		82 - search		114 - written
*;19 - upload		51 - not		83 - eprom		115 - interface
*;20 - paulmon		52 - for		84 - next		116 - install
*;21 - or		53 - baud		85 - more		117 - checksum
*;22 - of		54 - when		86 - available		118 - instruction
*;23 - no		55 - rate		87 - help		119 - unchanged
*;24 - intel		56 - can		88 - edit		120 - end
*;25 - flash		57 - are		89 - well		121 - transfer
*;26 - external		58 - use		90 - user		122 - time
*;27 - errors		59 - serial		91 - dump		123 - any
*;28 - editing		60 - auto		92 - delays		124 - skip
*;29 - digits		61 - port		93 - these		125 - name
*;30 - command		62 - all		94 - terminal		126 - address
*;31 - begin		63 - make		95 - system		127 - print
*common_words:
*	db	0x82, 0x90, 0xE8, 0x23, 0x86, 0x05, 0x4C, 0xF8
*	db	0x44, 0xB3, 0xB0, 0xB1, 0x48, 0x5F, 0xF0, 0x11
*	db	0x7F, 0xA0, 0x15, 0x7F, 0x1C, 0x2E, 0xD1, 0x40
*	db	0x5A, 0x50, 0xF1, 0x03, 0xBF, 0xBA, 0x0C, 0x2F
*	db	0x96, 0x01, 0x8D, 0x3F, 0x95, 0x38, 0x0D, 0x6F
*	db	0x5F, 0x12, 0x07, 0x71, 0x0E, 0x56, 0x2F, 0x48
*	db	0x3B, 0x62, 0x58, 0x20, 0x1F, 0x76, 0x70, 0x32
*	db	0x24, 0x40, 0xB8, 0x40, 0xE1, 0x61, 0x8F, 0x01
*	db	0x34, 0x0B, 0xCA, 0x89, 0xD3, 0xC0, 0xA3, 0xB9
*	db	0x58, 0x80, 0x04, 0xF8, 0x02, 0x85, 0x60, 0x25
*	db	0x91, 0xF0, 0x92, 0x73, 0x1F, 0x10, 0x7F, 0x12
*	db	0x54, 0x93, 0x10, 0x44, 0x48, 0x07, 0xD1, 0x26
*	db	0x56, 0x4F, 0xD0, 0xF6, 0x64, 0x72, 0xE0, 0xB8
*	db	0x3B, 0xD5, 0xF0, 0x16, 0x4F, 0x56, 0x30, 0x6F
*	db	0x48, 0x02, 0x5F, 0xA8, 0x20, 0x1F, 0x01, 0x76
*	db	0x30, 0xD5, 0x60, 0x25, 0x41, 0xA4, 0x2C, 0x60
*	db	0x05, 0x6F, 0x01, 0x3F, 0x26, 0x1F, 0x30, 0x07
*	db	0x8E, 0x1D, 0xF0, 0x63, 0x99, 0xF0, 0x42, 0xB8
*	db	0x20, 0x1F, 0x23, 0x30, 0x02, 0x7A, 0xD1, 0x60
*	db	0x2F, 0xF0, 0xF6, 0x05, 0x8F, 0x93, 0x1A, 0x50
*	db	0x28, 0xF0, 0x82, 0x04, 0x6F, 0xA3, 0x0D, 0x3F
*	db	0x1F, 0x51, 0x40, 0x23, 0x01, 0x3E, 0x05, 0x43
*	db	0x01, 0x7A, 0x01, 0x17, 0x64, 0x93, 0x30, 0x2A
*	db	0x08, 0x8C, 0x24, 0x30, 0x99, 0xB0, 0xF3, 0x19
*	db	0x60, 0x25, 0x41, 0x35, 0x09, 0x8E, 0xCB, 0x19
*	db	0x12, 0x30, 0x05, 0x1F, 0x31, 0x1D, 0x04, 0x14
*	db	0x4F, 0x76, 0x12, 0x04, 0xAB, 0x27, 0x90, 0x56
*	db	0x01, 0x2F, 0xA8, 0xD5, 0xF0, 0xAA, 0x26, 0x20
*	db	0x5F, 0x1C, 0xF0, 0xF3, 0x61, 0xFE, 0x01, 0x41
*	db	0x73, 0x01, 0x27, 0xC1, 0xC0, 0x84, 0x8F, 0xD6
*	db	0x01, 0x87, 0x70, 0x56, 0x4F, 0x19, 0x70, 0x1F
*	db	0xA8, 0xD9, 0x90, 0x76, 0x02, 0x17, 0x43, 0xFE
*	db	0x01, 0xC1, 0x84, 0x0B, 0x15, 0x7F, 0x02, 0x8B
*	db	0x14, 0x30, 0x8F, 0x63, 0x39, 0x6F, 0x19, 0xF0
*	db	0x11, 0xC9, 0x10, 0x6D, 0x02, 0x3F, 0x91, 0x09
*	db	0x7A, 0x41, 0xD0, 0xBA, 0x0C, 0x1D, 0x39, 0x5F
*	db	0x07, 0xF2, 0x11, 0x17, 0x20, 0x41, 0x6B, 0x35
*	db	0x09, 0xF7, 0x75, 0x12, 0x0B, 0xA7, 0xCC, 0x48
*	db	0x02, 0x3F, 0x64, 0x12, 0xA0, 0x0C, 0x27, 0xE3
*	db	0x9F, 0xC0, 0x14, 0x77, 0x70, 0x11, 0x40, 0x71
*	db	0x21, 0xC0, 0x68, 0x25, 0x41, 0xF0, 0x62, 0x7F
*	db	0xD1, 0xD0, 0x21, 0xE1, 0x62, 0x58, 0xB0, 0xF3
*	db	0x05, 0x1F, 0x73, 0x30, 0x77, 0xB1, 0x6F, 0x19
*	db	0xE0, 0x19, 0x43, 0xE0, 0x58, 0x2F, 0xF6, 0xA4
*	db	0x14, 0xD0, 0x23, 0x03, 0xFE, 0x31, 0xF5, 0x14
*	db	0x30, 0x99, 0xF8, 0x03, 0x3F, 0x64, 0x22, 0x51
*	db	0x60, 0x25, 0x41, 0x2F, 0xE3, 0x01, 0x56, 0x27
*	db	0x93, 0x09, 0xFE, 0x11, 0xFE, 0x79, 0xBA, 0x60
*	db	0x75, 0x42, 0xEA, 0x62, 0x58, 0xA0, 0xE5, 0x1F
*	db	0x53, 0x4F, 0xD1, 0x10, 0xd5, 0x20, 0x34, 0x75
*	db	0x2f, 0x41, 0x20, 0xb6, 0x01, 0x53, 0x5f, 0x70
*	db	0x9f, 0xc6, 0x50, 0xb3, 0x01, 0xd3, 0x4d, 0x71
*	db	0x07, 0x4c, 0x56, 0x02
*
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
*
*;str_logon1:		db	"Welcome",128," z80Mon v0.1",13,14			; Welcome string (OLD)
*;str_logon2:	 	db	32,32,"See",148,"2.DOC,",148,"2.EQU",164
*;			db	148,"2.HDR",180,213,141,".",14				; Documentation string
*str_logon1:		db	"Welcome",128," z80Mon",14				; Welcome string
*
*;str_prompt1:		db	148,"2 Loc:",0						; Paulmon2 Loc: (OLD)
*str_prompt1:		db	"z80Mon:",0						; z80Mon:
*str_prompt2:		db	" >", 160						;  > abort run which program(	(must follow after prompt1)
*;str_prompt3:		db	134,202,130,'(',0					; run which program(
*;;str_prompt4:		db	"),",149,140,128,200,": ",0				; ), or esc to quit: (OLD)
*str_prompt4:		db	",",149,31,140,": ",0					; , or Escape:
*;str_prompt5:		db	31,151,130,195,"s",199,166,131,","
*			db	186," JUMP",128,134,161,"r",130,13,14			; No program headers found in memory, use JUMP to run your program
*str_prompt6:		db	13,13,31,135,131,129,": ",0				; \n\nNew memory location:
*str_prompt7:		db	31,228,251," key: ",0					; Press any key:
*;str_prompt8:		db	13,13,31,136,128,131,129," (",0				; \n\nJump to memory location ( (OLD)
*str_prompt8:		db	13,13,31,136,128,131,129,0				; \n\nJump to memory location
*;str_prompt9:		db	13,13,31,130,31,253,0					; \n\nProgram Name (OLD)
*str_prompt9:		db	13,31,130,31,253,0					; \nProgram Name
*str_prompt9b:		db	31,129,32,32,32,32,32,31,201,14				; Location      Type	 (must follow prompt9)
*str_prompt10:		db	") ",31,135,31,178,": ",0				; ) New Value:
*str_prompt11:		db	31,189,": ",0						; Port:
*str_prompt12:		db	31,178,": ",0						; Value:
*str_prompt14:		db	13,13,31,135,227,129,": ",0				; \n\nNew stack location:
*str_prompt15:		db	31,228,251," key",180,212," page,",149,140,128,200,14	; Press any key for next page, or esc to quit
*
*str_type1:		db	31,154,158,0						; External command
*str_type2:		db	31,130,0						; Program
*str_type3:		db	"Init",0						; Init
*str_type4:		db	31,143,31,226,31,170,0					; Start Up Code
*str_type5:		db	"???",0							; ???
*
*str_tag_help2:		db	31,215,0						; Help
*str_tag_help1: 		db	31,142,215,209,0					; This help list (these 11 _cmd string must be in order)
*str_tag_listm:  	db	31,209,130,"s",0					; List Programs
*;str_tag_run:  		db	31,134,130,0						; Run Program
*str_tag_dnld: 		db	31,138,0						; Download
*str_tag_upld: 		db	31,147,0						; Upload
*str_tag_nloc: 		db	31,135,129,0						; New Location
*str_tag_jump: 		db	31,136,128,131,129,0					; Jump to memory location
*;str_tag_dump: 		db	31,132,219,154,131,0					; Hex dump external memory (OLD)
*str_tag_hexdump: 	db	31,132,219,131,0					; Hex dump memory
*str_tag_in:		db	"Read",166,189,0					; Read in port
*str_tag_out:		db	31,225,128,189,0					; Write to port
*str_tag_regdump: 	db	31,219,31,196,"s",0					; Dump Registers
*;str_tag_edit: 		db	31,156,154,146,0					; Editing external ram (OLD)
*str_tag_edit: 		db	31,216,31,146,0						; Edit Ram
*str_tag_clrmem: 	db	31,237,131,0						; Clear memory
*str_tag_stack:		db	31,240,227,129,0					; Change stack location
*
*str_help1:		db	13,13,"Standard",31,158,"s",14				; \n\nStandard Commands
*str_help2:		db	31,218,31,244,"ed",31,158,"s",14			; User Installed Commands
*;str_abort:		db	" ",31,158,31,160,"!",13,14				;  Command Abort!\n\n
*str_abort:		db	" ",31,158,31,160,"!",14				;  Command Abort!\n
*;str_runs:		db	13,134,"ning",130,":",13,14				; \nRunning program:\n\n
*str_runs:		db	13,134,"ning",130," @",0				; \nRunning program @
*
*;str_edit1: 		db	13,13,31,156,154,146,",",140,128,200,14			; \n\nEditing external ram, esc to quit\n (OLD)
*str_edit1: 		db	13,13,31,156,31,146,",",31,140,128,200,14		; \n\nEditing Ram, Esc to quit\n
*str_edit2: 		db	"  ",31,156,193,",",142,129,247,13,14			;   Editing complete, this location unchanged\n\n
*
*str_start_addr:		db	31,143,31,254,": ",0
*str_end_addr:		db	31,248,31,254,": ",0
*str_sure:		db	31,185,161," sure?",0					; Are you sure?
*str_clrcomp:		db	31,131,237,193,14					; Memory clear complete\n
*
*str_invalid:		db	"Invalid selection",14
*
*str_upld1: 		db	13,13,"Sending",31,152,132,137,172,32,32,0		; \n\nSending Intel hex file from
*str_upld2:		db	" to ",0						; to
*;str_upld2: 		db	" ",128,32,32,0						;  to
*
*str_dnld1: 		db	13,13,31,159," ascii",249,150,31,152,132,137		; \n\nBegin ascii transfer of Intel hex file
*			db	",",149,140,128,160,13,14				; , or esc to abort \n\n
*str_dnld2: 		db	13,31,138,160,"ed",13,14				; \nDownload aborted\n\n
*str_dnld3: 		db	13,31,138,193,"d",13,14					; \nDownload completed\n\n
*str_dnld4: 		db	13,"Summary:",14					; \nSummary:\n
*str_dnld5: 		db	" ",198,"s",145,"d",14					;  lines received\n
*str_dnld6a:		db	" ",139,145,"d",14					;  bytes received\n
*str_dnld6b:		db	" ",139," written",14					;  bytes written\n
*str_dnld7: 		db	31,155,":",14						; Errors:\n
*str_dnld8: 		db	" ",139," unable",128," write",14			;  bytes unable to write\n
*str_dnld9: 		db	32,32,"bad",245,"s",14					; bad checksums\n
*str_dnld10:		db	" ",133,159,150,198,14					;  unexpected begin of line\n
*str_dnld11:		db	" ",133,132,157,14					;  unexpected hex digits\n
*str_dnld12:		db	" ",133," non",132,157,14				;  unexpected non hex digits\n
*str_dnld13:		db	31,151,155," detected",13,14				; No errors detected\n\n
*
*str_ny:			db	" (N/y): ",0
*
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
