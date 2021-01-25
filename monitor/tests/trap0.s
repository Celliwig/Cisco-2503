###########################################################################
#                                                                         #
#              Test exception handling by executing TRAP0                 #
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
	.asciz	"TRAP0"

.org	0x40
#	move.l	#0x0, %a0						/* Set VBR */
#	movec	%a0, %VBR

	movea.l	#0x00000080, %a4					/* Address of TRAP0 exception handler in EVT */
	lea.l	trap0_handler, %a5					/* Address of new TRAP0 exception handler */
	move.l	%a5, (%a4)						/* Save new exception handler */

	jsr	print_newline
	trap	#0							/* Execute TRAP 0 */
	jsr	print_newline

	move.l	#0xffff, %d0						/* Setup pause counter */
trap0_loop:
	dbra	%d0, trap0_loop						/* Pause */

	rts

trap0_handler:
	lea	str_trap, %a0						/* Load pointer to string */
	jsr	print_str_simple					/* Pritn string */
	rte

str_trap:	.asciz	"It's a TRAP!"

.org	0x100

# print_newline
#################################
#  Print a new line
print_newline:
	mov.b	#'\r', %d0
	jsr	console_out
	mov.b	#'\n', %d0
	jsr	console_out
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


