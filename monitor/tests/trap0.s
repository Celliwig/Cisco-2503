###########################################################################
#                                                                         #
#                     Test TRAP0 (console) routines                       #
#                                                                         #
###########################################################################

.include "../cisco-2500/scn2681.h"
.equiv	console_out, scn2681_out_A
.equiv	console_in, scn2681_in_A
.include	"../include/trapdefs.h"

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
trap0_test:
/* Console routines */
	/* Print 'Hello' */
	mov.b	#TRAP0_CONOUT, %d7					/* Console out */
	mov.b	#'H', %d0
	trap	#0							/* Execute TRAP 0 */
	mov.b	#'e', %d0
	trap	#0							/* Execute TRAP 0 */
	mov.b	#'l', %d0
	trap	#0							/* Execute TRAP 0 */
	mov.b	#'l', %d0
	trap	#0							/* Execute TRAP 0 */
	mov.b	#'o', %d0
	trap	#0							/* Execute TRAP 0 */
	mov.b	#'\n', %d0
	trap	#0							/* Execute TRAP 0 */
	mov.b	#'\r', %d0
	trap	#0							/* Execute TRAP 0 */

	/* Read character in */
	mov.b	#TRAP0_CONIN, %d7					/* Console in */
trap0_test_char_in:
	trap	#0
	cmp.b	#' ', %d0
	bne.s	trap0_test_char_in

/* Print routines */
	mov.b	#TRAP0_PRNT_STR, %d7
	lea	str_simple_test, %a0
	trap	#0

	mov.b	#TRAP0_PRNT_RSTR, %d7
	lea	str_setup_border_top, %a0
	trap	#0
	mov.b	#TRAP0_PRNT_NEWLINE, %d7
	trap	#0
	mov.b	#TRAP0_PRNT_RSTR, %d7
	lea	str_setup_border_middle, %a0
	trap	#0
	mov.b	#TRAP0_PRNT_NEWLINE, %d7
	trap	#0
	mov.b	#TRAP0_PRNT_RSTR, %d7
	lea	str_setup_border_bottom, %a0
	trap	#0
	mov.b	#TRAP0_PRNT_NEWLINE, %d7
	trap	#0

	mov.b	#TRAP0_PRNT_HEX32, %d7
	mov.l	#0xdeadbeef, %d0
	trap	#0
	mov.b	#TRAP0_PRNT_NEWLINE, %d7
	trap	#0

	move.l	#0xffff, %d0						/* Setup pause counter */
trap0_test_loop:
	dbra	%d0, trap0_test_loop					/* Pause */

	rts

str_simple_test:		.asciz		"Simple string test\r\n"
str_setup_border_top:		dc.b		0x01,0xc9,0x0c,0xcd,0x01,0xcb,0x2d,0xcd,0x01,0xbb,0x00
str_setup_border_middle:	dc.b		0x01,0xba,0x0c,0x20,0x01,0xba,0x2d,0x20,0x01,0xba,0x00
str_setup_border_bottom:	dc.b		0x01,0xc8,0x0c,0xcd,0x01,0xca,0x2d,0xcd,0x01,0xbc,0x00
