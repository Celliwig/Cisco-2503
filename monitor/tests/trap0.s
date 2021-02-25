###########################################################################
#                                                                         #
#                     Test TRAP0 (console) routines                       #
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
trap0_test:
	/* Print 'Hello' */
	move.b	#0x0, %d7						/* Console out */
	move.b	#'H', %d0
	trap	#0							/* Execute TRAP 0 */
	move.b	#'e', %d0
	trap	#0							/* Execute TRAP 0 */
	move.b	#'l', %d0
	trap	#0							/* Execute TRAP 0 */
	move.b	#'l', %d0
	trap	#0							/* Execute TRAP 0 */
	move.b	#'o', %d0
	trap	#0							/* Execute TRAP 0 */
	move.b	#'\n', %d0
	trap	#0							/* Execute TRAP 0 */
	move.b	#'\r', %d0
	trap	#0							/* Execute TRAP 0 */

	/* Read character in */
	move.b	#1, %d7							/* Console in */
trap0_test_char_in:
	trap	#0
	cmp.b	#' ', %d0
	bne.s	trap0_test_char_in

	move.l	#0xffff, %d0						/* Setup pause counter */
trap0_test_loop:
	dbra	%d0, trap0_test_loop					/* Pause */

	rts
