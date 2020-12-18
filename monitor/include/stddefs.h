# Version
#################################
.equiv	M68KMON_VERSION_MAJOR, 0x00
.equiv	M68KMON_VERSION_MINOR, 0x01
.equiv	M68KMON_VERSION, M68KMON_VERSION_MINOR<<16 | M68KMON_VERSION_MAJOR

.include "include/ascii.h"

# Builtin command keys
###########################################################################
.equiv			command_key_help, '?'					/* Help screen */
.equiv			command_key_listm, 'M'					/* Directory list */
#.equiv			command_key_run, 'P'					/* Run program */
.equiv			command_key_download, 'D'				/* Download */
.equiv			command_key_upload, 'U'					/* Upload */
.equiv			command_key_new_locat, 'N'				/* New memory location */
.equiv			command_key_jump, 'J'					/* Jump to memory location */
.equiv			command_key_hexdump, 'H'				/* Hex dump memory */
.equiv			command_key_regdump, 'R'				/* Dump register data */
.equiv			command_key_edit, 'E'					/* Edit memory */
.equiv			command_key_clrmem, 'C'					/* Clear memory */
.equiv			command_key_stack, 'S'					/* Set stack */

# Monitor configuration bits
###########################################################################
.equiv			MONITOR_CONFIG_PRINT, 0					/* If set, print character */

/* Provides the Exception Vector Table */
.include "include/evt.h"
