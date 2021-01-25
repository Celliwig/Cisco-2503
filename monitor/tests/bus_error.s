###########################################################################
#                                                                         #
#            Test exception handling by creating a bus error              #
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
	.asciz	"Bus Error"

.org	0x40
	movea.l	#8, %a0							/* Address of bus error exception handler in EVT */
	lea.l	bus_error_handler, %a1					/* Address of new bus error exception handler */
	move.l	%a1, (%a0)						/* Save new exception handler */
	movea.l	#0x012000ff, %a0					/* Load A0 with an address which should cause a bus error */
	move.l	(%a0), %d0						/* Read from that address to cause exception */
	rts

bus_error_handler:
	move.l	#0xabad1dea, %d0
	andi.w	#0xceff, (0xa, %a7)					/* Clear re-run flags */
	rte
