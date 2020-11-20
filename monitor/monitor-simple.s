*-----------------------------------------------------------
* Title      : Monitor-Simple
* Description: Based on https://github.com/ChartreuseK/68k-Monitor
*-----------------------------------------------------------
*
* To make this responsive to different terminal widths we need to change the number of bytes printed
* on a line from 16, which fits exactly on a 72 column screen, to an ammount based on a formula.
*  Sizes:
*   Address:      "000000: " 8
*   Each Byte:    "00 "      3
*   Start ASCII:  "|"        1
*   Each ASCII:   "."        1
*   End ASCII:    "|"        1
*
*   Width = 8 + numBytes*(3 + 1) + 2
*   numBytes = (Width - 10)/4 = (Width - 10)>>2
*  Examples:
*    (80 - 10)/4 = 70/4 = 16 Bytes
*    (40 - 10)/4 = 30/4 =  7 Bytes
*    (32 - 10)/4 = 22/4 =  5 Bytes
* On small screens we should not show the start and end characters on the ASCII section
* 40 Characters wide or less
*    (40 - 8)/4  = 32/4 =  8 Bytes
*    (32 - 8)/4  = 24/4 =  6 Bytes


**********************************
* Defines
*
RAM_START		= 0x00000000
/* RAM_END			= 0x200000 */
RAM_END			= 0x1000
MAX_LINE_LENGTH		= 80

*********************************
* 68681 Duart Register Addresses
*
DUART			= 0x02120100			/* Base Addr of DUART */
MRA			= DUART + 0			/* Mode Register A		(R/W) */
SRA			= DUART + 1			/* Status Register A		(R) */
CSRA			= DUART + 1			/* Clock Select Register A	(W) */
CRA			= DUART + 2			/* Commands Register A		(W) */
RBA			= DUART + 3			/* Receiver Buffer A		(R) */
TBA			= DUART + 3			/* Transmitter Buffer A		(W) */
ACR			= DUART + 4			/* Aux. Control Register	(R/W) */
ISR			= DUART + 5			/* Interrupt Status Register	(R) */
IMR			= DUART + 5			/* Interrupt Mask Register	(W) */
MRB			= DUART + 8			/* Mode Register B		(R/W) */
SRB			= DUART + 9			/* Status Register B		(R) */
CSRB			= DUART + 9			/* Clock Select Register B	(W) */
CRB			= DUART + 10			/* Commands Register B		(W) */
RBB			= DUART + 11			/* Reciever Buffer B		(R) */
TBB			= DUART + 11			/* Transmitter Buffer B		(W) */
/* IVR			= DUART + 12			/* Interrupt Vector Register	(R/W) */ 

**********************************
* ASCII Control Characters

BEL			= 0x07
BKSP			= 0x08				/* CTRL-H */
TAB			= 0x09
LF			= 0x0A
CR			= 0x0D
ESC			= 0x1B

CTRLC			= 0x03
CTRLX			= 0x18				/* Line Clear */

**********************************
* Variables

varCurAddr		= RAM_END - 4			/* Last address accessed */
varLineBuf		= varCurAddr-MAX_LINE_LENGTH-2	/* Line buffer */
varLast			= varLineBuf
STACK_START		= varLast

monitor_start		= 0x1000			/* Start of monitor code */
bootrom_start		= 0x01000000			/* Start of boot ROM */

/* Exception Vector Table */
	.org	0x00000000
/* FIRST 8 bytes loaded after reset */
	dc.l	STACK_START				/* Supervisor stack pointer */
	dc.l	bootrom_start + monitor_start		/* Initial PC */


	.org	monitor_start
********************************************
* Initialisation
init:
	move.l	#0x3e8, %d0				/* Whether this delay is necessary */
init_delay_loop:
	subq.l	#0x1, %d0				/* Don't know, added anyway */
	bgt.w	init_delay_loop

	movea.l	#0x2110000, %a0				/* Swap out boot ROM */
	move.w	(%a0), %d0
	andi.w	#0xfffe, %d0
	move.w	%d0, (%a0)

	move.l	#STACK_START, %A0			/* Set our stack pointer to be sure */
	move.l	%A0, %SP
	jsr	initDuart				/* Setup the serial port */

********************************************
* Simple Ram Readback Test
*
ramCheck:
	lea	msgRamCheck, %A0
	bsr.w	printString
	lea	RAM_START, %A2
ramCheck_loop:
	move.b	#$AA, (%A2)				/* First test with 10101010 */
	cmp.b	#$AA, (%A2)
	bne.s	ramCheck_fail
	move.b	#0x55, (%A2)				/* Then with 01010101 */
	cmp.b	#0x55, (%A2)
	bne.s	ramCheck_fail
	move.b	#0x00, (%A2)				/* And finally clear the memory */
	cmp.b	#0x00, (%A2)+				/* And move to the next byte */
	bne.s	ramCheck_fail
	cmp.l	#RAM_END, %A2
	blt.s	ramCheck_loop				/* While we're still below the end of ram to check */
	bra.s	ramCheck_succ
ramCheck_fail:						/* One of the bytes of RAM failed to readback test */
	lea	msgRamFail, %A0
	bsr.w	printString
	move.l	%A2, %D0
	bsr.w	printHexLong 				/* Print out the address that failed */
	bsr.w	printNewline
ramCheck_haltloop:					/* Sit forever in the halt loop */
	bra.s	ramCheck_haltloop
ramCheck_succ:						/* All bytes passed the readback test */
	lea	msgRamPass, %A0
	bsr.w	printString

**************************************************
* Warm Restart entry point
*
monitorStart:
	lea	msgBanner, %A0				/* Show our banner */
	bsr.w	printString
	lea	msgHelp, %A0				/* And the command help message */
	bsr.w	printString

monitorLine:						/* Our main monitor loop */
	lea	msgPrompt, %a0				/* Prompt */
	bsr.w	printString
	bsr.w	readLine				/* Read in the line */
	bsr.w	lineToUpper				/* Convert to upper-case for ease of parsing */
	bsr.w	parseLine				/* Then parse and respond to the line */
	bra.s	monitorLine

***************************************
* Converts input line to uppercase
lineToUpper:
	lea	varLineBuf, %a0				/* Get the start of the line buffer */
lineToUpper_loop:
	move.b	(%a0), %d0				/* Read in a character */
	cmp.b	#'a', %d0
	blt.s	lineToUpper_next			/* Is it less than lower-case 'a', then move on */
	cmp.b	#'z', %d0
	bgt.s	lineToUpper_next			/* Is it greater than lower-case 'z', then move on */
	sub.b	#0x20, %d0				/* Then convert a to A, b to B, etc. */
lineToUpper_next:
	move.b	%d0, (%a0)+				/* Store the character back into a0, and move to the next */
	bne.s	lineToUpper_loop			/* Keep going till we hit a null terminator */
	rts

***************************************
* Parse Line
parseLine:
	movem.l	%a2-%a3, -(%SP)				/* Save registers */
	lea	varLineBuf, %a0
parseLine_findCommand:
	move.b	(%a0)+, %d0
	cmp.b	#' ', %d0				/* Ignore spaces */
	beq.w	parseLine_findCommand
	cmp.b	#'E', %d0				/* Examine command */
	beq.w	function_examine
	cmp.b	#'D', %d0				/* Deposit command */
	beq.w	function_deposit
	cmp.b	#'R', %d0				/* Run command */
	beq.w	function_run
	cmp.b	#'H', %d0				/* Help command */
	beq.w	print_help
	cmp.b	#0, %d0					/* Ignore blank lines */
	beq.s	parseLine_finish
parseLine_invalid:
	lea	msgInvalidCommand, %a0
	bsr.w	printString
parseLine_finish:
	movem.l	(%SP)+, %a2-%a3				/* Restore registers */
	rts

**********************
* Examines memory addresses
* Valid modes:
*   e ADDR                  Displays a single byte
*   e ADDR-ADDR             Dispalys all bytes between the two addresses
*   e ADDR+LEN              Dispays LEN bytes after ADDR
*   e ADDR;                 Interactive mode, space shows 16 lines, enter shows 1.
*   e ADDR.                 Quick line, displays one line
function_examine:
	bsr.w	parseNumber			/* Read in the start address */
	tst.b	%d1				/* Make sure it's valid (parseNumber returns non-zero in d1 for failure) */
	bne.w	print_invalidAddr
	move.l	%d0, %a3			/* Save the start address */
function_examine_exloop:
	move.b	(%a0)+, %d0
	cmp.b	#' ', %d0			/* Ignore spaces */
	beq.s	function_examine_exloop
	cmp.b	#'-', %d0			/* Check if it's a range specifier */
	beq.s	function_examine_exrange
	cmp.b	#'+', %d0			/* Check if it's a length specifier */
	beq.s	function_examine_exlength
	cmp.b	#';', %d0			/* Check if we're going interactive */
	beq.s	function_examine_exinter
	cmp.b	#'.', %d0			/* Check if quick 16 */
	beq.s	function_examine_exquick
	move.l	#1, %d0				/* Otherwise read in a single byte */
	bra.s	function_examine_exend
function_examine_exrange:
	bsr.w	parseNumber			/* Find the end address */
	tst.b	%d1				/* Check if we found a valid address */
	bne.w	print_invalidAddr
	sub.l	%a3, %d0			/* Get the length */
	bra.s	function_examine_exend
function_examine_exquick:			/* Quick mode means show one line of 16 bytes */
	move.l	#0x10, %d0
	bra.s	function_examine_exend
function_examine_exlength:			/* Length mode means a length is specified */
	bsr.w	parseNumber			/* Find the length */
	tst.b	%d1
	bne.w	print_invalidAddr
function_examine_exend:				/* We're done parsing, give the parameters to dumpRAM and exit */
	move.l	%a3, %a0
	bsr.w	dumpRAM
	bra.s	parseLine_finish
function_examine_exinter:			/* Interactive mode, Space shows 16 lines, enter shows 1. */
	move.l	%a3, %a0			/* Current Address */
	move.l	#0x10, %d0			/* 16 bytes */
	bsr.w	dumpRAM				/* Dump this line */
	add.l	#0x10, %a3			/* Move up the current address 16 bytes */
function_examine_exinterend:
	bsr.w	inChar
	cmp.b	#CR, %d0			/* Display another line */
	beq.s	function_examine_exinter
	cmp.b	#' ', %d0			/* Display a page (256 bytes at a time) */
	beq.s	function_examine_exinterpage
	bra.s	parseLine_finish		/* Otherwise exit */
function_examine_exinterpage:
	move.l	%a3, %a0
	move.l	#0x100, %d0			/* 256 bytes */
	bsr.w	dumpRAM				/* Dump 16 lines of RAM */
	add.l	#0x100, %a3			/* Move up the current address by 256 */
	bra.s	function_examine_exinterend

****************************************
* Deposit values into RAM
* d ADDR VAL VAL            Deposit value(s) into RAM
* d ADDR VAL VAL;           Deposit values, continue with values on next line
*  VAL VAL VAL;              - Continuing with further continue
* d: VAL VAL                Continue depositing values after the last address written to
function_deposit:
	move.b	(%a0), %d0
	cmp.b	#':', %d0			/* Check if we want to continue from last */
	beq.s	function_deposit_depCont
	bsr.w	parseNumber			/* Otherwise read the address */
	tst.b	%d1
	bne.s	print_invalidAddr
	move.l	%d0, %a3			/* Save the start address */
function_deposit_depLoop:
	move.b	(%a0), %d0
	cmp.b	#';', %d0			/* Check for continue */
	beq.s	function_deposit_depMultiline
	tst	%d0				/* Check for the end of line */
	beq	function_deposit_depEnd
	bsr.s	parseNumber			/* Otherwise read a value */
	tst.b	%d1
	bne.s	print_invalidVal
	cmp.w	#255, %d0			/* Make sure it's a byte */
	bgt.s	print_invalidVal
	move.b	%d0, (%a3)+			/* Store the value into memory */
	bra.s	function_deposit_depLoop
function_deposit_depCont:
	move.l	varCurAddr, %a3			/* Read in the last address */
	addq.l	#1, %a0				/* Skip over the ':' */
	bra.s	function_deposit_depLoop
function_deposit_depMultiline:
	lea	msgDepositPrompt, %a0
	bsr.w	printString
	bsr.w	readLine			/* Read in the next line to be parsed */
	bsr.w	lineToUpper			/* Convert to uppercase */
	lea	varLineBuf, %a0			/* Reset our buffer pointer */
	bra.s	function_deposit_depLoop	/* And jump back to decoding */
function_deposit_depEnd:
	move.l	%a3, varCurAddr
	bra.w	parseLine_finish

*****************************************
*
function_run:
	bsr.w	parseNumber			/* Otherwise read the address */
	tst.b	%d1
	bne.s	print_invalidAddr
	move.l	%d0, %a0
	jsr	(%a0)				/* Jump to the code! */
						/* Go as subroutine to allow code to return to us */
	jsr	monitorStart			/* Warm start after returning so everything is in */
						/* a known state. */

print_help:
	lea	msgHelp, %a0
	bsr.w	printString
	bra.w	parseLine_finish
print_invalidAddr:
	lea	msgInvalidAddress, %a0
	bsr.w	printString
	bra.w	parseLine_finish
print_invalidVal:
	lea	msgInvalidValue, %a0
	bsr.w	printString
	bra.w	parseLine_finish

**************************************
* Find and parse a hex number
*  Starting address in A0
*  Number returned in D0
*  Status in D1   (0 success, 1 fail)
*  TODO: Try and merge first digit code with remaining digit code
parseNumber:
	eor.l	%d0, %d0			/* Zero out d0 */
	move.b	(%a0)+, %d0
	cmp.b	#' ', %d0			/* Ignore all leading spaces */
	beq.s	parseNumber
	cmp.b	#'0', %d0			/* Look for hex digits 0-9 */
	blt.s	parseNumber_invalid
	cmp.b	#'9', %d0
	ble.s	parseNumber_firstdigit1
	cmp.b	#'A', %d0			/* Look for hex digits A-F */
	blt.s	parseNumber_invalid
	cmp.b	#'F', %d0
	ble.s	parseNumber_firstdigit2
parseNumber_invalid:
	move.l	#1, %d1				/* Invalid character, mark failure and return */
	rts
parseNumber_firstdigit2:
	sub.b	#'7', %d0			/* Turn 'A' to 10 */
	bra.s	parseNumber_loop
parseNumber_firstdigit1:
	sub.b	#'0', %d0			/* Turn '0' to 0 */
parseNumber_loop:
	move.b	(%a0)+, %d1			/* Read in a digit */
	cmp.b	#'0', %d1			/* Look for hex digits 0-9 */
	blt.s	parseNumber_finish		/* Any other characters mean we're done reading */
	cmp.b	#'9', %d1
	ble.s	parseNumber_digit1
	cmp.b	#'A', %d1			/* Look for hex digits A-F */
	blt.s	parseNumber_finish
	cmp.b	#'F', %d1
	ble.s	parseNumber_digit2
parseNumber_finish:				/* We hit a non-hex digit character, we're done parsing */
	subq.l	#1, %a0				/* Move the pointer back before the end character we read */
	move.l	#0, %d1
	rts
parseNumber_digit2:
	sub.b	#'7', %d1			/* Turn 'A' to 10 */
	bra.s	parseNumber_digit3
parseNumber_digit1:
	sub.b	#'0', %d1			/* Turn '0' to 0 */
parseNumber_digit3:
	lsl.l	#4, %d0				/* Shift over to the next nybble */
	add.b	d1, %d0				/* Place in our current nybble (could be or.b instead) */
	bra.s	parseNumber_loop

****************************************
* Dumps a section of RAM to the screen
* Displays both hex values and ASCII characters
* d0 - Number of bytes to dump
* a0 - Start Address
dumpRAM:
	movem.l	%d2-%d4/%a2, -(%SP)		/* Save registers */
	move.l	%a0, %a2			/* Save the start address */
	move.l	%d0, %d2			/* And the number of bytes */
dumpRAM_line:
	move.l	%a2, %d0
	bsr.w	printHexAddr			/* Starting address of this line */
	lea	msgColonSpace, %a0
	bsr.w	printString
	move.l	#16, %d3			/* 16 Bytes can be printed on a line */
	move.l	%d3, %d4			/* Save number of bytes on this line */
dumpRAM_hexbyte:
	tst.l	%d2				/* Check if we're out of bytes */
	beq.s	dumpRAM_endbytesShort
	tst.b	%d3				/* Check if we're done this line */
	beq.s	dumpRAM_endbytes
	move.b	(%a2)+, %d0			/* Read a byte in from RAM */
	bsr.w	printHexByte			/* Display it */
	move.b	#' ', %d0
	bsr.w	outChar				/* Space out bytes */
	subq.l	#1, %d3
	subq.l	#1, %d2
	bra.s	dumpRAM_hexbyte
dumpRAM_endbytesShort:
	sub.b	%d3, %d4			/* Make d4 the actual number of bytes on this line */
	move.b	#' ', %d0
dumpRAM_endbytesShortLoop:
	tst.b	%d3				/* Check if we ended the line */
	beq.s	dumpRAM_endbytes
	move.b	#' ', %d0
	bsr.w	outChar				/* Three spaces to pad out */
	move.b	#' ', %d0
	bsr.w	outChar
	move.b	#' ', %d0
	bsr.w	outChar
	subq.b	#1, %d3
	bra.s	dumpRAM_endbytesShortLoop
dumpRAM_endbytes:
	suba.l	%d4, %a2				/* Return to the start address of this line */
dumpRAM_endbytesLoop:
	tst.b	%d4				/* Check if we're done printing ascii */
	beq	dumpRAM_endline
	subq.b	#1, %d4
	move.b	(%a2)+, %d0			/* Read the byte again */
	cmp.b	#' ', %d0			/* Lowest printable character */
	blt.s	dumpRAM_unprintable
	cmp.b	#'~', %d0			/* Highest printable character */
	bgt.s	dumpRAM_unprintable
	bsr.w	outChar
	bra.s	dumpRAM_endbytesLoop
dumpRAM_unprintable:
	move.b	#'.', %d0
	bsr.w	outChar
	bra.s	dumpRAM_endbytesLoop
dumpRAM_endline:
	lea	msgNewline, %a0
	bsr.w	printString
	tst.l	%d2
	ble.s	dumpRAM_finish
	bra.w	dumpRAM_line
dumpRAM_finish:
	movem.l	(%SP)+, %d2-%d4/%a2		/* Restore registers */
	rts

******
* Read in a line into the line buffer
readLine:
	movem.l	%d2/%a2, -(%SP)			/* Save changed registers */
	lea	varLineBuf, %a2			/* Start of the lineBuffer */
	eor.w	%d2, %d2			/* Clear the character counter */
readLine_loop:
	bsr.w	inChar				/* Read a character from the serial port */
	cmp.b	#BKSP, %d0			/* Is it a backspace? */
	beq.s	readLine_backspace
	cmp.b	#CTRLX, %d0			/* Is it Ctrl-H (Line Clear)? */
	beq.s	readLine_lineclear
	cmp.b	#CR, %d0			/* Is it a carriage return? */
	beq.s	readLine_endline
	cmp.b	#LF, %d0			/* Is it anything else but a LF? */
	beq.s	readLine_loop			/* Ignore LFs and get the next character */
readLine_char:					/* Normal character to be inserted into the buffer */
	cmp.w	#MAX_LINE_LENGTH, %d2
	bge.s	readLine_loop			/* If the buffer is full ignore the character */
	move.b	%d0, (%a2)+			/* Otherwise store the character */
	addq.w	#1, %d2				/* Increment character count */
	bsr.w	outChar				/* Echo the character */
	bra.s	readLine_loop			/* And get the next one */
readLine_backspace:
	tst.w	%d2				/* Are we at the beginning of the line? */
	beq.s	readLine_loop			/* Then ignore it */
	bsr.w	outChar				/* Backspace */
	move.b	#' ', %d0
	bsr.w	outChar				/* Space */
	move.b	#BKSP, %d0
	bsr.w	outChar				/* Backspace */
	subq.l	#1, %a2				/* Move back in the buffer */
	subq.l	#1, %d2				/* And current character count */
	bra.s	readLine_loop			/* And goto the next character */
readLine_lineclear:
	tst	%d2				/* Anything to clear? */
	beq.s	readLine_loop			/* If not, fetch the next character */
	suba.l	%d2, %a2			/* Return to the start of the buffer */
readLine_lineclearloop:
	move.b	#BKSP, %d0
	bsr.w	outChar				/* Backspace */
	move.b	#' ', %d0
	bsr.w	outChar				/* Space */
	move.b	#BKSP, %d0
	bsr.w	outChar				/* Backspace */
	subq.w	#1, %d2
	bne.s	readLine_lineclearloop		/* Go till the start of the line */
	bra.s	readLine_loop
readLine_endline:
	bsr.w	outChar				/* Echo the CR */
	move.b	#LF, %d0
	bsr.w	outChar				/* Line feed to be safe */
	move.b	#0, (%a2)			/* Terminate the line (Buffer is longer than max to allow this at full length) */
	movea.l	%a2, %a0			/* Ready the pointer to return (if needed) */
	movem.l	(%SP)+, %d2/%a2			/* Restore registers */
	rts					/* And return */

******
* Prints a newline (CR, LF)
printNewline:
	lea	msgNewline, %a0
******
* Print a null terminated string
printString:
printString_loop:
	move.b	(%a0)+, %d0			/* Read in character */
	beq.s	printString_finish		/* Check for the null */
	bsr.s	outChar				/* Otherwise write the character */
	bra.s	printString_loop		/* And continue */
printString_finish:
	rts

** KEEP All printHex functions together **
******
* Print a hex word
printHexWord:
	move.l	%d2, -(%SP)			/* Save D2 */
	move.l	%d0, %d2			/* Save the address in d2 */
	rol.l	#8, %d2				/* 4321 -> 3214 */
	rol.l	#8, %d2				/* 3214 -> 2143 */
	bra.s	printHex_wordentry		/* Print out the last 16 bits */
*****
* Print a hex 24-bit address
printHexAddr:
	move.l	%d2, -(%SP)			/* Save D2 */
	move.l	%d0, %d2			/* Save the address in d2 */
	rol.l	#8, %d2				/* 4321 -> 3214 */
	bra.s	printHex_addrentry		/* Print out the last 24 bits */
******
* Print a hex long
printHexLong:
	move.l	%d2, -(%SP)			/* Save D2 */
	move.l	%d0, %d2			/* Save the address in d2 */
	rol.l	#8, %d2				/* 4321 -> 3214 high byte in low */
	move.l	%d2, %d0
	bsr.s	printHexByte			/* Print the high byte (24-31) */
printHex_addrentry:
	rol.l	#8, %d2				/* 3214 -> 2143 middle-high byte in low */
	move.l	%d2, %d0
	bsr.s	printHexByte			/* Print the high-middle byte (16-23) */
printHex_wordentry:
	rol.l	#8, %d2				/* 2143 -> 1432 Middle byte in low */
	move.l	%d2, %d0
	bsr.s	printHexByte			/* Print the middle byte (8-15) */
	rol.l	#8, %d2
	move.l	%d2, %d0
	bsr.s	printHexByte			/* Print the low byte (0-7) */
	move.l	(%SP)+, %d2			/* Restore D2 */
	rts

******
* Print a hex byte
*  - Takes byte in D0
printHexByte:
	move.l	%D2, -(%SP)
	move.b	%D0, %D2
	lsr.b	#4, %D0
	add.b	#'0', %D0
	cmp.b	#'9', %D0			/* Check if the hex number was from 0-9 */
	ble.s	printHexByte_digit2
	add.b	#7, %D0				/* Shift 0xA-0xF from ':' to 'A' */
printHexByte_digit2:
	bsr.s	outChar				/* Print the digit */
	andi.b	#0x0F, %D2			/* Now we want the lower digit Mask only the lower digit */
	add.b	#'0', %D2
	cmp.b	#'9', %D2			/* Same as before */
	ble.s	printHexByte_finish
	add.b	#7, %D2
printHexByte_finish:
	move.b	%D2, %D0
	bsr.s	outChar				/* Print the lower digit */
	move.l	(%SP)+, %D2
	rts

*****
* Writes a character to Port A, blocking if not ready (Full buffer)
*  - Takes a character in D0
outChar:
	btst    #2, SRA				/* Check if transmitter ready bit is set */
	beq     outChar
	move.b  %d0, TBA			/* Transmit Character */
	rts

*****
* Reads in a character from Port A, blocking if none available
*  - Returns character in D0
*
inChar:
	btst	#0, SRA				/* Check if receiver ready bit is set */
	beq	inChar
	move.b	RBA, %d0			/* Read Character into D0 */
	rts

*****
* Initializes the 68681 DUART port A as 9600 8N1
initDuart:
	move.b	#0x30, CRA			/* Reset Transmitter */
	move.b	#0x20, CRA			/* Reset Reciever */
	move.b	#0x10, CRA			/* Reset Mode Register Pointer */
	move.b	#0x80, ACR			/* Baud Rate Set #2 */
	move.b	#0xBB, CSRA			/* Set Tx and Rx rates to 9600 */
	move.b	#0x93, MRA			/* 7-bit, No Parity ($93 for 8-bit, $92 for 7-bit) */
	move.b	#0x07, MRA			/* Normal Mode, Not CTS/RTS, 1 stop bit */
	move.b	#0x05, CRA			/* Enable Transmit/Recieve */
	rts


**********************************
* Strings
*
msgBanner:
	.ascii	"\r\nSimple 68000 ROM Monitor\r\n"
	.asciz	"==============================\r\n"
msgHelp:
	.ascii	"Available Commands: \r\n"
	.asciz	" (E)xamine    (D)eposit    (R)un     (H)elp\r\n"
msgDepositPrompt:
	.asciz	": "
msgPrompt:
	.asciz	"> "
msgInvalidCommand:
	.asciz	"Invalid Command\r\n"
msgInvalidAddress:
	.asciz	"Invalid Address\r\n"
msgInvalidValue:
	.asciz	"Invalid Value\r\n"
msgRamCheck:
	.asciz	"Checking RAM, Esc to cancel...\r\n"
msgRamFail:
	.asciz	"Failed at: "
msgRamPass:
	.asciz	"Passed.\r\n"
msgNewline:
	.asciz	"\r\n"
msgColonSpace:
	.asciz	": "


	.end	START			/* last line of source */
