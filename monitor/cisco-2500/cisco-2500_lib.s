.include "cisco-2500/cisco-2500.h"

/* Cisco 2500 library offset from monitor start */
CISCO2500_LIB = monitor_start + 0x4000

.org CISCO2500_LIB

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

	/* Swap out boot ROM */
	movea.l	#0x2110000, %a0
	move.w	(%a0), %d0
	andi.w	#0xfffe, %d0
	move.w	%d0, (%a0)

	jmp	(%a7)							/* Return address is stored in A7 */

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

*; ###########################################################################
*; #                                                                         #
*; #                             System Config                               #
*; #                                                                         #
*; ###########################################################################
*
*orgmem  nc100_cmd_base+0x0200
*	db	0xA5,0xE5,0xE0,0xA5					; signiture bytes
*	db	254,'!',0,0						; id (254=cmd)
*	db	0,0,0,0							; prompt code vector
*	db	0,0,0,0							; reserved
*	db	0,0,0,0							; reserved
*	db	0,0,0,0							; reserved
*	db	0,0,0,0							; user defined
*	db	255,255,255,255						; length and checksum (255=unused)
*	db	"System config",0
*
*orgmem  nc100_cmd_base+0x0240
*include	"nc100/setup.asm"

