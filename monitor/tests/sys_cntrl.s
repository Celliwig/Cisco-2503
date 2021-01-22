###########################################################################
#                                                                         #
#                     Test system control registers                       #
#                                                                         #
###########################################################################

.org	0x0
	dc.b	0xA5,0xE5,0xE0,0xA5                                     /* signiture bytes */
	dc.b	254,'!',0,0                                             /* id (254=cmd) */
	dc.b	0,0,0,0                                                 /* prompt code vector */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* reserved */
	dc.b	0,0,0,0                                                 /* user defined */
	dc.b	255,255,255,255                                         /* length and checksum (255=unused) */
	.asciz	"System Ctrl Regs"

.org	0x40
	lea	2110000, %a0						/* Load address of System Control Register 1 */
	move.l	(%a0), %d0						/* Get current value */
	ori.l	#0x10, %d0						/* Set bit */
	move.l	%d0, (%a0)						/* Update register */
	rts
