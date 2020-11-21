# Serial config routines
###########################################################################
.include "cisco-2500/scn2681.h"

scn2681_init:
	mov.b	#0x30, SCN2681_ADDR_WR_COMMAND_A	/* Reset Transmitter */
	mov.b	#0x20, SCN2681_ADDR_WR_COMMAND_A	/* Reset Reciever */
	mov.b	#0x10, SCN2681_ADDR_WR_COMMAND_A	/* Reset Mode Register Pointer */
	mov.b	#0x80, SCN2681_ADDR_WR_AUX_CONTROL	/* Baud Rate Set #2 */
	mov.b	#0xBB, SCN2681_ADDR_WR_CLOCK_SELECT_A	/* Set Tx and Rx rates to 9600 */
	mov.b	#0x93, SCN2681_ADDR_WR_MODE_A		/* 8-bit, No Parity ($93 for 8-bit, $92 for 7-bit) */
	mov.b	#0x07, SCN2681_ADDR_WR_MODE_A		/* Normal Mode, Not CTS/RTS, 1 stop bit */
	mov.b	#0x05, SCN2681_ADDR_WR_COMMAND_A	/* Enable Transmit/Recieve */
        rts


*; # nc100_serial_init
*; #################################
*;  From power on (state unknown), reset UART, turn off line driver
*nc100_serial_init:
*	xor	a							; Clear A
*	out	(nc100_uart_control_register), a			; Three writes of 0x0
*	call	nc100_serial_setup_delay
*	out	(nc100_uart_control_register), a			; Clears the way
*	call	nc100_serial_setup_delay
*	out	(nc100_uart_control_register), a			; For a new command bytes
*	call	nc100_serial_setup_delay
*; # nc100_serial_reset_actual
*; #################################
*;  Reset UART, turn off line driver
*nc100_serial_reset_actual:
*	; Software reset UART
*	ld	a, uPD71051_reg_commask_SRes				; Software reset
*	out	(nc100_uart_control_register), a 			; Write command byte
*	call	nc100_serial_setup_delay
*
*	; Reset UART/ turn off line driver
*	ld	a, nc100_serial_clk_rst | nc100_serial_line_driver	; Value to apply: UART/Line driver off
*	ld	b, 0xe0							; Retain: Memcard register and parallel strobe
*	call	nc100_io_misc_config_A_write				; Write value to I/O port
*	call	nc100_serial_setup_delay
*	ld	a, (nc100_config_uart_baud)
*	res	nc100_config_uart_baud_on, a				; Clear flag indicating UART/Line driver on
*	ld	(nc100_config_uart_baud), a
*	ret
*; # nc100_serial_reset
*; #################################
*;  Reset UART, turn off line driver
*;  if permitted.
*nc100_serial_reset:
*	ld	a, (nc100_config_uart_baud)
*	bit	nc100_config_uart_baud_always, a			; Test if UART should always be enabled
*	jr	z, nc100_serial_reset_actual
*	ret

*; # nc100_serial_config
*; #################################
*;  Setup UART using config information.
*;  Turn on line driver
*nc100_serial_config:
*	ld	bc, (nc100_config_uart_mode)				; B = Baud rate (lower nibble), C = UART mode byte
*	bit	3, b							; Test if 38400 (not a directly supported mode)
*	jr	z, nc100_serial_config_actual				; It's not, so just config UART
*	ld	b, 0x04							; Using baud 2400
*	ld	a, c							; Update mode byte
*	and	0xfc							; Remove baud rate clock
*	or	uPD71051_reg_mode_bclk_x1				; Select x1 baud rate clock
*	ld	c, a							; Save new mode byte
*; # nc100_serial_config_actual
*; #################################
*;  Setup UART, turn on line driver
*;	In:	B = Baud rate
*;		C = Mode configuration
*nc100_serial_config_actual:
*	; Config UART/ turn on line driver
*	ld	a, b							; Copy baud rate
*	and	0x07							; Filters baud rate value
*	ld	b, 0xe0							; Retain: Memcard register and parallel strobe
*	call	nc100_io_misc_config_A_write				; Write value to I/O port
*	call	nc100_serial_setup_delay
*
*	ld	a, c							; Copy mode configuration
*	out	(nc100_uart_control_register), a 			; Write mode to control register
*	call	nc100_serial_setup_delay
*
*	; Write command byte to UART
*	ld	a, uPD71051_reg_commask_full
*	out	(nc100_uart_control_register), a 			; Write command byte
*	call	nc100_serial_setup_delay
*
*	ld	a, (nc100_config_uart_baud)
*	set	nc100_config_uart_baud_on, a				; Mark as UART on
*	ld	(nc100_config_uart_baud), a
*
*	ret
*
*; # nc100_serial_always_enabled
*; #################################
*;  This overrides any calls to shutdown the serial subsystem.
*;  The one exception to this is when the UART configuration is changed.
*;  However once the new configuration is loaded, the UART is enabled
*;  once again.
*nc100_serial_always_enabled:
*	ld	a, (nc100_config_uart_baud)				; Get UART baud config
*	set	nc100_config_uart_baud_always, a
*	ld	(nc100_config_uart_baud), a				; Save UART baud config
*	ret

*nc100_serial_always_clear:
*	ld	a, (nc100_config_uart_baud)				; Get UART baud config
*	res	nc100_config_uart_baud_always, a
*	ld	(nc100_config_uart_baud), a				; Save UART baud config
*	ret

*nc100_serial_always_toggle:
*	ld	a, (nc100_config_uart_baud)				; Get UART baud config
*	xor	1 << nc100_config_uart_baud_always
*	ld	(nc100_config_uart_baud), a				; Save UART baud config
*	ret

*; # nc100_serial_baud_dec
*; #################################
*nc100_serial_baud_dec:
*	ld	a, (nc100_config_uart_baud)				; Get UART baud config
*	ld	b, a
*	and	0xf0							; Clear baud bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	0x0f							; Get baud bits
*	jr	nz, nc100_serial_baud_dec_do				; Check if at zero
*	ld	a, 0x08							; Reset value if zero
*	jr	nc100_serial_baud_dec_end
*nc100_serial_baud_dec_do:
*	dec	a							; Decrement selected baud
*	and	0x0f							; Get baud bits
*nc100_serial_baud_dec_end:
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_baud), a				; Save UART mode config
*	ret

*; # nc100_serial_baud_inc
*; #################################
*nc100_serial_baud_inc:
*	ld	a, (nc100_config_uart_baud)				; Get UART baud config
*	ld	b, a
*	and	0xf0							; Clear baud bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	0x0f							; Get baud bits
*	cp	0x08							; Check if reached end of baud list
*	jr	nz, nc100_serial_baud_inc_do
*	xor	a							; Reset baud value
*	jr	nc100_serial_baud_inc_end
*nc100_serial_baud_inc_do:
*	inc	a							; Increment selected baud
*	and	0x0f							; Get baud bits
*nc100_serial_baud_inc_end:
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_baud), a				; Save UART mode config
*	ret

*; # nc100_serial_character_length_dec
*; #################################
*nc100_serial_character_length_dec:
*	ld	a, (nc100_config_uart_mode)				; Get UART mode config
*	ld	b, a
*	and	0xff^uPD71051_reg_mode_chrlen_mask			; Clear character length bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	uPD71051_reg_mode_chrlen_mask				; Get character length bits
*	sub	0x04							; Decrement character length
*	and	uPD71051_reg_mode_chrlen_mask				; Get character length bits
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_mode), a				; Save UART mode config
*	ret

*; # nc100_serial_character_length_inc
*; #################################
*nc100_serial_character_length_inc:
*	ld	a, (nc100_config_uart_mode)				; Get UART mode config
*	ld	b, a
*	and	0xff^uPD71051_reg_mode_chrlen_mask			; Clear character length bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	uPD71051_reg_mode_chrlen_mask				; Get character length bits
*	add	0x04							; Increment character length
*	and	uPD71051_reg_mode_chrlen_mask				; Get character length bits
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_mode), a				; Save UART mode config
*	ret

*; # nc100_serial_parity_dec
*; #################################
*nc100_serial_parity_dec:
*	ld	a, (nc100_config_uart_mode)				; Get UART mode config
*	ld	b, a
*	and	0xff^uPD71051_reg_mode_parity_mask			; Clear parity bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	uPD71051_reg_mode_parity_mask				; Get parity bits
*	sub	0x10							; Decrement
*	and	uPD71051_reg_mode_parity_mask				; Get parity bits
*	cp	0x20							; Check for annoying gap
*	jr	nz, nc100_serial_parity_dec_end
*	ld	a, 0x10
*nc100_serial_parity_dec_end:
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_mode), a				; Save UART mode config
*	ret

*; # nc100_serial_parity_inc
*; #################################
*nc100_serial_parity_inc:
*	ld	a, (nc100_config_uart_mode)				; Get UART mode config
*	ld	b, a
*	and	0xff^uPD71051_reg_mode_parity_mask			; Clear parity bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	uPD71051_reg_mode_parity_mask				; Get parity bits
*	add	0x10							; Increment
*	and	uPD71051_reg_mode_parity_mask				; Get parity bits
*	cp	0x20							; Check for annoying gap
*	jr	nz, nc100_serial_parity_inc_end
*	ld	a, 0x30
*nc100_serial_parity_inc_end:
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_mode), a				; Save UART mode config
*	ret

*; # nc100_serial_stopbits_dec
*; #################################
*nc100_serial_stopbits_dec:
*	ld	a, (nc100_config_uart_mode)				; Get UART mode config
*	ld	b, a
*	and	0xff^uPD71051_reg_mode_stopbit_mask			; Clear stopbits bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	uPD71051_reg_mode_stopbit_mask				; Get stopbits bits
*	sub	0x40							; Decrement
*	and	uPD71051_reg_mode_stopbit_mask				; Get stopbits bits
*	cp	0x00							; Check for annoying gap
*	jr	nz, nc100_serial_stopbits_dec_end
*	ld	a, 0xc0
*nc100_serial_stopbits_dec_end:
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_mode), a				; Save UART mode config
*	ret

*; # nc100_serial_stopbits_inc
*; #################################
*nc100_serial_stopbits_inc:
*	ld	a, (nc100_config_uart_mode)				; Get UART mode config
*	ld	b, a
*	and	0xff^uPD71051_reg_mode_stopbit_mask			; Clear stopbits bits
*	ld	c, a							; Store for later
*	ld	a, b
*	and	uPD71051_reg_mode_stopbit_mask				; Get stopbits bits
*	add	0x40							; Increment
*	and	uPD71051_reg_mode_stopbit_mask				; Get stopbits bits
*	cp	0x00							; Check for annoying gap
*	jr	nz, nc100_serial_stopbits_inc_end
*	ld	a, 0x40
*nc100_serial_stopbits_inc_end:
*	or	c							; Combine with saved value
*	ld	(nc100_config_uart_mode), a				; Save UART mode config
*	ret
