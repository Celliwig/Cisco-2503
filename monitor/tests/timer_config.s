###########################################################################
#                                                                         #
#            Test exception handling by creating a bus error              #
#                                                                         #
###########################################################################

.include "../cisco-2500/scn2681.h"
.equiv  console_out, scn2681_out_A
.equiv  console_in, scn2681_in_A
.equiv  console_in_nocheck, scn2681_in_A_nocheck

.org	0x0
	dc.b	0xA5,0xE5,0xE0,0xA5                                     /* signiture bytes */
	dc.b	254,'!',0,0                                             /* id (254=cmd) */
	dc.b	0,0,0,0                                                 /* prompt code vector */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* user defined */
	dc.b	255,255,255,255                                         /* length and checksum (255=unused) */
	.asciz	"Timer Config"

.org	0x40
timer_init:
	clr.l	%d0
	lea	time_reg_l, %a0
	move.l	%d0, (%a0)						/* Clear lower time store */
	lea	time_reg_h, %a0
	move.l	%d0, (%a0)						/* Clear high time store */
	movea.l	#0x7c, %a0						/* Address of NMI handler in EVT */
	lea.l	nmi_handler, %a1					/* Address of new NMI handler */
	move.l	%a1, (%a0)						/* Save new exception handler */

timer_cisco_C:
	move.w	#0xfa0, 0x2120070.l
	move.w	#0x3e80, 0x2120050.l
	movea.l #0x2120040, %a0
	clr.b	(%a0)
	move.b	(%a0), %d0						/* Clear interrupt flag */

timer_read_loop:
	move.b	#'\r', %d0
	jsr	console_out

	lea	str_timer, %a0
	jsr	print_str_simple
	lea	time_reg_h, %a0
	move.l	(%a0), %d3
	jsr	print_hex32
	move.b	#':', %d0
	jsr	console_out
	lea	time_reg_l, %a0
	move.l	(%a0), %d3
	jsr	print_hex32

	jsr	console_in_nocheck
	cmp.b	#' ', %d0
	bne.s	timer_read_loop

	rts

nmi_handler:
	movem.l	%d0-%d1/%a0-%a1, -(%a7)
	move.b	0x2120040.l, %d0					/* Clear interrupt flag */
	move.w	#0x3e80, 0x2120050.l

	lea	time_reg_l, %a0						/* Get addresses of time store */
	lea	time_reg_h, %a1
	move.l	(%a0), %d0						/* Get lower long of time */
	addq.l	#4, %d0							/* Increment time */
	move.l	%d0, (%a0)						/* Write back */
	bne.s	nmi_handler_finish					/* Finish if no rollover */
	move.l	(%a1), %d0						/* Get high long of time */
	addq.l	#1, %d0							/* Increment time */
	move.l	%d0, (%a1)						/* Write back */

nmi_handler_finish:
	movem.l	(%a7)+, %d0-%d1/%a0-%a1
	rte

time_reg_l:	dc.l	0x00000000
time_reg_h:	dc.l	0x00000000
str_timer:	.asciz	"Timer value: "

.org	0x120
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


