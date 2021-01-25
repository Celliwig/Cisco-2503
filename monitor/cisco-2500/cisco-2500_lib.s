.include "cisco-2500/cisco-2500.h"

/* Cisco 2500 library offset from monitor start */
CISCO2500_LIB = monitor_start + 0x2000

.org CISCO2500_LIB

# Serial device
.include "cisco-2500/scn2681_config.s"
.include "cisco-2500/scn2681_io.s"


###########################################################################
#                                                                         #
#                               System Init                               #
#                                                                         #
###########################################################################

.org CISCO2500_LIB + 0x0100
	dc.b	0xA5,0xE5,0xE0,0xA5					/* signiture bytes */
	dc.b	249,',',0,0						/* id (249=init) */
	dc.b	0,0,0,0							/* prompt code vector */
	dc.b	0,0,0,0							/* reserved */
	dc.b	0,0,0,0							/* reserved */
	dc.b	0,0,0,0							/* reserved */
	dc.b	0,0,0,0							/* user defined */
	dc.b	255,255,255,255						/* length and checksum (255=unused) */
	.asciz	"System init"

.org	CISCO2500_LIB + 0x140						/* executable code begins here */
system_init:
	/* Don't know if this is necessary */
	/* Was in the original firmware */
	move.l	#0x3e8, %d0
system_init_delay_loop:
	subq.l	#0x1, %d0
	bgt.w	system_init_delay_loop

	/* Set/Reset VBR */
	move.l  #0x0, %a0
	movec   %a0, %VBR

	/* Swap out boot ROM */
	movea.l	#0x2110000, %a0
	move.w	(%a0), %d0
	andi.w	#0xfffe, %d0
	move.w	%d0, (%a0)

	/* Copy exception vector table */
	mov.l	#0x01000000, %a0
	mov.l	#0x010003ff, %a1
	mov.l	#0x00000000, %a2
	jsr	memory_copy

	jmp	(%a6)							/* Return address is stored in A7 */

###########################################################################
#                                                                         #
#                            Startup Command                              #
#                                                                         #
###########################################################################

.org CISCO2500_LIB + 0x0200
	dc.b	0xA5,0xE5,0xE0,0xA5					/* signiture bytes */
	dc.b	253,',',0,0						/* id (249=init) */
	dc.b	0,0,0,0							/* prompt code vector */
	dc.b	0,0,0,0							/* reserved */
	dc.b	0,0,0,0							/* reserved */
	dc.b	0,0,0,0							/* reserved */
	dc.b	0,0,0,0							/* user defined */
	dc.b	255,255,255,255						/* length and checksum (255=unused) */
	.asciz	"Startup Command"

.org CISCO2500_LIB + 0x0240						/* executable code begins here */
startup_cmd:
	# Configure variables
	mov.l	#0x00001000, MONITOR_ADDR_CURRENT			/* Set current address */

	jsr	scn2681_init						/* Init DUART console */
startup_cmd_console_clear:						/* Drain existing data from console UART */
	jsr	scn2681_in_A_nocheck
	jsr	scn2681_in_A_check					/* Check if there's any more data */
	bne.s	startup_cmd_console_clear				/* Loop until buffer clear */

	/* Detect and configure EPROM & RAM */
	# Need to write
	#movea.l #0x2110002, %a0						/* System config register 2 */

	rts

startup_cmd_config:
*	; Check for device config reset
*	call	nc100_keyboard_char_in					; Get key (if there is one)
*	cp	character_code_delete					; Check if delete
*	jr	nz, startup_cmd_config_load
*startup_cmd_config_load_defaults:
*	call	nc100_config_load_defaults				; Restore and apply default system config
*	jr	startup_cmd_continue
*startup_cmd_config_load:
*	call	nc100_config_restore					; Restore and apply system config
*
*startup_cmd_continue:
*	; Configure z80Mon variables
*	ld	bc, 0x4000
*	ld	(z80mon_default_addr), bc				; Set monitor's current address: 0x4000
*
*	rst	16							; Continue boot

############################################################################
##                                                                         #
##                             System Config                               #
##                                                                         #
############################################################################
#
#.org CISCO2500_LIB + 0x0300
#	dc.b	0xA5,0xE5,0xE0,0xA5					/* signiture bytes */
#	dc.b	254,'!',0,0						/* id (254=cmd) */
#	dc.b	0,0,0,0							/* prompt code vector */
#	dc.b	0,0,0,0							/* reserved */
#	dc.b	0,0,0,0							/* reserved */
#	dc.b	0,0,0,0							/* reserved */
#	dc.b	0,0,0,0							/* user defined */
#	dc.b	255,255,255,255						/* length and checksum (255=unused) */
#	.asciz	"System config"
#
#.org CISCO2500_LIB + 0x0340
#.include	"nc100/setup.asm"

