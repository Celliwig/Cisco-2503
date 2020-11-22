# Version
#################################
.equiv	m68kmon_version_major, 0x00
.equiv	m68kmon_version_minor, 0x01
.equiv	m68kmon_version, m68kmon_version_minor<<16 | m68kmon_version_major

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


/* Provides the Exception Vector Table */
.include "include/evt.h"
