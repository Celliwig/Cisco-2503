# Version
#################################
.equiv	m68kmon_version_major, 0x00
.equiv	m68kmon_version_minor, 0x01
.equiv	m68kmon_version, m68kmon_version_minor<<16 | m68kmon_version_major

.include "include/ascii.h"

/* Provides the Exception Vector Table */
.include "include/evt.h"
