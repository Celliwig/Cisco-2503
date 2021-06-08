###########################################################################
#                                                                         #
#              Test access to Boot Flash command registers                #
#                                                                         #
###########################################################################

# Routine from Cisco source
###########################################################################
# 011169de: 4e56 0000           : link    A6, #$0
# 011169e2: 2f0a                : move.l  A2, -(A7)
# 011169e4: 2f02                : move.l  D2, -(A7)
# 011169e6: 206e 0008           : movea.l ($8,A6), A0
# 011169ea: 226e 000c           : movea.l ($c,A6), A1
# 011169ee: 246e 0010           : movea.l ($10,A6), A2
# 011169f2: 342e 0016           : move.w  ($16,A6), D2
# 011169f6: 223c 0001 5554      : move.l  #$15554, D1
# 011169fc: 21bc 5555 5555 1800 : move.l  #$55555555, (A0,D1.l)
# 01116a04: 203c 0000 aaa8      : move.l  #$aaa8, D0
# 01116a0a: 21bc aaaa aaaa 0800 : move.l  #$aaaaaaaa, (A0,D0.l)
# 01116a12: 21bc 0909 0909 1800 : move.l  #$9090909, (A0,D1.l)
# 01116a1a: 4a42                : tst.w   D2
# 01116a1c: 6704                : beq     $1116a22
# 01116a1e: 5342                : subq.w  #1, D2
# 01116a20: 66fc                : bne     $1116a1e
# 01116a22: 2290                : move.l  (A0), (A1)
# 01116a24: 5848                : addq.w  #4, A0
# 01116a26: 2490                : move.l  (A0), (A2)
# 01116a28: 20bc 0f0f 0f0f      : move.l  #$f0f0f0f, (A0)
# 01116a2e: 20bc 0f0f 0f0f      : move.l  #$f0f0f0f, (A0)
# 01116a34: 242e fff8           : move.l  (-$8,A6), D2
# 01116a38: 246e fffc           : movea.l (-$4,A6), A2
# 01116a3c: 4e5e                : unlk    A6
# 01116a3e: 4e75                : rts


.include "../cisco-2500/scn2681.h"
.equiv  console_out, scn2681_out_A
.equiv  console_in, scn2681_in_A
.equiv  console_in_nocheck, scn2681_in_A_nocheck
.equiv	console_in_check, scn2681_in_A_check
.equiv  ascii_linefeed, 0x0a						/* Line Feed ASCII code */
.equiv  ascii_carriage_return, 0x0d					/* Carriage Return ASCII code */

.equiv	flash_addr1, 0x555<<1						/* Device command register address 1 */
.equiv	flash_addr2, 0x2AA<<1						/* Device command register address 2 */
									/* Addresses shifted because line A0 not connected */
.equiv	flash_cmd1, 0x5555						/* Autoselect Command: #1 (AA reversed) */
.equiv	flash_cmd2, 0xAAAA						/* Autoselect Command: #2 (55 reversed) */
.equiv	flash_cmd3, 0x0909						/* Autoselect Command: #3 (90 reversed) */

.org	0x0
	dc.b	0xA5,0xE5,0xE0,0xA5                                     /* signiture bytes */
	dc.b	254,'!',0,0                                             /* id (254=cmd) */
	dc.b	0,0,0,0                                                 /* prompt code vector */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* user defined */
	dc.b	255,255,255,255                                         /* length and checksum (255=unused) */
	.asciz	"Boot Flash Test"

.org	0x40
flash_boot_test:
	movea.l	#0x01000000, %a0					/* Boot Flash ROM base address */
	move.w	#0x2329, %d2						/* Delay loop value */
	move.l	#flash_addr1, %d1
	move.w	#flash_cmd1, (%a0,%d1.l)				/* 1st command */
	move.l	#flash_addr2, %d0
	move.w	#flash_cmd2, (%a0,%d0.l)				/* 2nd command */
	move.w	#flash_cmd3, (%a0,%d1.l)				/* 3rd command */
	tst.w	%d2
	beq.s	flash_boot_test_get_rslt
flash_boot_test_delay_loop:
	subq.w	#1, %d2
	bne.s	flash_boot_test_delay_loop
flash_boot_test_get_rslt:
	move.w	(%a0), %d6
	addq.w	#2, %a0
	move.w	(%a0), %d7
	move.l	#0xf0f0f0f, (%a0)					/* Reset command (x2???) */
	move.l	#0xf0f0f0f, (%a0)

	lea	str_boot, %a0
	jsr	print_str_simple
	lea	str_flash, %a0
	jsr	print_str_simple
	lea	str_manu, %a0
	jsr	print_str_simple
	lea	str_id, %a0
	jsr	print_str_simple

	move.b	%d6, %d0						/* Print manufacturer ID */
	jsr	reverse_byte_bits
	jsr	print_hex8
	jsr	print_newline

	lea	str_boot, %a0
	jsr	print_str_simple
	lea	str_flash, %a0
	jsr	print_str_simple
	lea	str_device, %a0
	jsr	print_str_simple
	lea	str_id, %a0
	jsr	print_str_simple

	move.b	%d7, %d0						/* Print device ID */
	jsr	reverse_byte_bits
	jsr	print_hex8
	jsr	print_newline

	move.w	#0xffff, %d0
flash_boot_finish_loop:
	dbra	%d0, flash_boot_finish_loop

	rts

# reverse_byte_bits
#################################
#  Flash data bus wired 'backwards'
#  Does not affect normal operation
#  Flash commands and results must be processed
#	In:	D0 = Byte to be processed
#	Out:	D1 = Processed byte
reverse_byte_bits:
	clr.l	%d1							/* Clear return register */
	move.w	#7, %d2							/* Number of times to loop */
reverse_byte_bits_loop:
	asl.b	#1, %d1							/* Shift result byte */
	asr.b	#1, %d0							/* Shift bit into Carry */
	bcc.s	reverse_byte_bits_loop_check				/* Don't add bit if Carry not set */
	ori.b	#1, %d1							/* Set bit in result */
reverse_byte_bits_loop_check:
	dbra	%d2, reverse_byte_bits_loop				/* Loop for remaining bits */
	rts

.org	0x120
str_device:	.asciz	"Device "
str_flash:	.asciz	"Flash "
str_id:		.asciz	"ID: "
str_boot:	.asciz	"Boot "
str_manu:	.asciz	"Manufacturer "

######################################################################################################################################################
# From m68kmon.s
######################################################################################################################################################

# Print routines
###########################################################################

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
