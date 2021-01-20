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

/* NVRAM */
#define	C2503_NVRAM_ADDR	0x02000000	/* NVRAM address */
#define	C2503_NVRAM_SIZE	0x20000		/* NVRAM window size (128k, maybe actually 32k) */
#define address_should_error	C2503_NVRAM_ADDR+C2503_NVRAM_SIZE

.org	0x40
	movea.l	address_should_error, %a0				/* Load A0 with an address which should cause a bus error */
	move.l	(%a0), %d0						/* Read from that address to cause exception */
	rts
