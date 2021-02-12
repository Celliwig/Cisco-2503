###########################################################################
#                                                                         #
#                             68302 init test                             #
#                                                                         #
###########################################################################

.include "../cisco-2500/scn2681.h"
.equiv  console_out, scn2681_out_A
.equiv  console_in, scn2681_in_A
.equiv  console_in_nocheck, scn2681_in_A_nocheck
.equiv	ascii_linefeed, 0x0a						/* Line Feed ASCII code */
.equiv	ascii_carriage_return, 0x0d					/* Carriage Return ASCII code */

.org	0x0
	dc.b	0xA5,0xE5,0xE0,0xA5                                     /* signiture bytes */
	dc.b	254,'!',0,0                                             /* id (254=cmd) */
	dc.b	0,0,0,0                                                 /* prompt code vector */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* user defined */
	dc.b	255,255,255,255                                         /* length and checksum (255=unused) */
	.asciz	"68302 test"

.org	0x40
mc68302_test:
	/* Replace IRQ4 handler */
	movea.l	#0x00000070, %a4					/* Address of IRQ4 exception handler in EVT */
	lea.l	irq4_handler, %a5					/* Address of new IRQ4 exception handler */
	move.l	%a5, (%a4)						/* Save new exception handler */

	/* Allow maskable level 4 interrupt */
	move	%sr, %d0
	and.l	#0xfbff, %d0
	move	%d0, %sr

	lea	0x2110004, %a0						/* Device control register */
	and.w	#0xfff7, (%a0)						/* Enable 68302 */

	/* Setup HD64570 timer 0 */
	move.w	#0xb001, 0x21000f2.l					/* Configure BAR */
	move.l	#0x3000ff, 0x21000f4.l					/* Configure SCR */

	lea	0x02101800, %a0
	move.w	#0x0fff, %d0
	clr.w	(0x84,%a0)						/* Disable SCC1 */
	move.w	%d0, (0x82,%a0)						/* SCON1 at lowest clock rate */
	clr.w	(0x94,%a0)						/* Disable SCC2 */
	move.w	%d0, (0x92,%a0)						/* SCON2 at lowest clock rate */
	clr.w	(0xa4,%a0)						/* Disable SCC3 */
	move.w	%d0, (0xa2,%a0)						/* SCON3 at lowest clock rate */

	move.w	#0x0000, (0xb0,%a0)					/* Disable SCP */

	move.w	#0x0000, (0x30,%a0)					/* Disable CS0 */
	move.w	#0x0000, (0x34,%a0)					/* Disable CS1 */
	move.w	#0x0000, (0x38,%a0)					/* Disable CS2 */
	move.w	#0x0000, (0x3c,%a0)					/* Disable CS3 */

	move.w	#0x0000, (0x40,%a0)					/* Disable Timer 1 */
	move.w	#0x0000, (0x4a,%a0)					/* Disable Watchdog */
	move.w	#0x0000, (0x50,%a0)					/* Timer 2: Reset */
	move.w	#0xffff, (0x52,%a0)					/* Timer 2: Set Reference */
	move.b	#0x3, (0x59,%a0)					/* Timer 2: Clear Event Register */
	move.w	#0xff1d, (0x50,%a0)					/* Timer 2: Enable */

	move.w	#0x7000, (0x12,%a0)					/* Global Interrupt Mode Register */
	move.w	#0x0040, (0x16,%a0)					/* Interrupt Mask Register (Timer 2 Interrupt) */

mc68302_test_finish:
	jsr	console_in_nocheck
	cmp.b	#' ', %d0
	bne.s	mc68302_test_finish

	rts

irq4_handler:
	movem.l %d0-%d1/%a0-%a1, -(%a7)

	/* Clear interrupt */
	lea	0x02101800, %a0
	move.b	#0x3, (0x59,%a0)					/* Timer 2: Clear Event Register */

	lea	str_ping, %a0
	jsr	print_str_simple
	jsr	print_newline

irq4_handler_finish:
	movem.l	(%a7)+, %d0-%d1/%a0-%a1
	rte

.org	0x120

str_ping:	.asciz	"Ping!"

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
