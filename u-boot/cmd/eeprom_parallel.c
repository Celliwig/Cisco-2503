// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Celliwig
 * Based on eeprom command by Wolfgang Denk
 */

/*
 * Support for read and write access to EEPROM like memory devices. This
 * includes regular EEPROM as well as  FRAM (ferroelectic nonvolaile RAM).
 * FRAM devices read and write data at bus speed. In particular, there is no
 * write delay. Also, there is no limit imposed on the number of bytes that can
 * be transferred with a single read or write.
 *
 * Use the following configuration options to ensure no unneeded performance
 * degradation (typical for EEPROM) is incured for FRAM memory:
 *
 * #define CONFIG_SYS_EEPROM_FRAM
 * #undef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
 *
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <eeprom.h>
#include <eeprom_layout.h>
#include <linux/delay.h>
#include <asm/io.h>

#ifndef CONFIG_SYS_EEPROM_BUS_WIDTH
#define CONFIG_SYS_EEPROM_BUS_WIDTH	8
#endif

#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	0
#endif

#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_BITS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	8
#endif

#define	EEPROM_PAGE_SIZE	(1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)
#define	EEPROM_PAGE_OFFSET(x)	((x) & (EEPROM_PAGE_SIZE - 1))

/*
 * Overridable function to enable EEPROM writes (ie. GPIO)
 */
__weak int eeprom_write_enable(unsigned dev_addr, int enable)
{
	return 0;
}

// Needed by env/eeprom.c
void eeprom_init(int ignore)
{
}

/*
 * Calculate the number of bytes that can be written
 * before the page address changes
 */
static int eeprom_write_len(unsigned offset, unsigned end)
{
	unsigned len = end - offset;

	/*
	 * For a FRAM device there is no limit on the number of the
	 * bytes that can be accessed with the single read or write
	 * operation.
	 */
#if !defined(CONFIG_SYS_EEPROM_FRAM)
	unsigned maxlen = EEPROM_PAGE_SIZE - EEPROM_PAGE_OFFSET(offset);

	if (len > maxlen)
		len = maxlen;
#endif

	return len;
}

static int eeprom_rw_block(unsigned dev_addr, unsigned offset,
			   uchar *buffer, unsigned len, bool read)
{
	int ret = 0;
	unsigned int last_write = 0;

	if (read) {
		// This could probably be simplified with memcpy
		// Need to check the effect of caching however
		if (CONFIG_SYS_EEPROM_BUS_WIDTH == 8) {
			while (len > 0) {
				*buffer = readb(dev_addr + offset);
				offset++;
				buffer++;
				len--;
			}
		}
	} else {
		if (CONFIG_SYS_EEPROM_BUS_WIDTH == 8) {
			while (len > 0) {
				writeb(*buffer, dev_addr + offset);
				last_write = *buffer;
				offset++;
				buffer++;
				len--;
			}

#ifdef CONFIG_SYS_EEPROM_LAST_WRITE_TEST
			// Some EEPROMs implement a system that reading the address of the last byte
			// written will provide an indication of the write status
			offset--;							// Undo last increment
			while (readb(dev_addr + offset) != last_write) {
				// Should probably implement a timeout here
			}
#endif
		}
	}

	return ret;
}

int eeprom_read(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	eeprom_init(-1);

	/*
	 * Read data until done.
	 */
	return eeprom_rw_block(dev_addr, offset, buffer, cnt, 1);
}

int eeprom_write(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned len;
	int ret;

	eeprom_init(-1);

	eeprom_write_enable(dev_addr, 1);

	/*
	 * Write data until done.
	 */
	while (offset < end) {
		// Get the largest number of bytes that can be written for this page
		len = eeprom_write_len(offset, end);

		ret = eeprom_rw_block(dev_addr, offset, buffer, len, 0);

		buffer += len;
		offset += len;

		// If there's no way to check the write status, delay
		if (CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS > 0)
			udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
	}

	eeprom_write_enable(dev_addr, 0);
	return ret;
}

static int parse_numeric_param(char *str)
{
	char *endptr;
	int value = simple_strtol(str, &endptr, 16);

	return (*endptr != '\0') ? -1 : value;
}

#ifdef CONFIG_CMD_EEPROM_LAYOUT

__weak int eeprom_parse_layout_version(char *str)
{
	return LAYOUT_VERSION_UNRECOGNIZED;
}

static unsigned char eeprom_buf[CONFIG_SYS_EEPROM_SIZE];

#endif

enum eeprom_action {
	EEPROM_READ,
	EEPROM_WRITE,
	EEPROM_PRINT,
	EEPROM_UPDATE,
	EEPROM_ACTION_INVALID,
};

static enum eeprom_action parse_action(char *cmd)
{
	if (!strncmp(cmd, "read", 4))
		return EEPROM_READ;
	if (!strncmp(cmd, "write", 5))
		return EEPROM_WRITE;
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (!strncmp(cmd, "print", 5))
		return EEPROM_PRINT;
	if (!strncmp(cmd, "update", 6))
		return EEPROM_UPDATE;
#endif

	return EEPROM_ACTION_INVALID;
}

static int eeprom_execute_command(enum eeprom_action action, ulong device_addr, int layout_ver,
				  char *key, char *value, ulong mem_addr, ulong off, ulong cnt)
{
	int rcode = 0;
	const char *const fmt = "\nEEPROM @0x%lX %s: addr 0x%08lx  off 0x%04lx  count %ld ... ";
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	struct eeprom_layout layout;
#endif

	if (action == EEPROM_ACTION_INVALID)
		return CMD_RET_USAGE;

	eeprom_init(-1);
	if (action == EEPROM_READ) {
		printf(fmt, device_addr, "read", mem_addr, off, cnt);

		rcode = eeprom_read(device_addr, off, (uchar *) mem_addr, cnt);

		puts("done\n");
		return rcode;
	} else if (action == EEPROM_WRITE) {
		printf(fmt, device_addr, "write", mem_addr, off, cnt);

		rcode = eeprom_write(device_addr, off, (uchar *) mem_addr, cnt);

		puts("done\n");
		return rcode;
	}

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	rcode = eeprom_read(device_addr, 0, eeprom_buf, CONFIG_SYS_EEPROM_SIZE);
	if (rcode < 0)
		return rcode;

	eeprom_layout_setup(&layout, eeprom_buf, CONFIG_SYS_EEPROM_SIZE, layout_ver);

	if (action == EEPROM_PRINT) {
		layout.print(&layout);
		return 0;
	}

	layout.update(&layout, key, value);

	rcode = eeprom_write(device_addr, 0, layout.data, CONFIG_SYS_EEPROM_SIZE);
#endif

	return rcode;
}

#define NEXT_PARAM(argc, index)	{ (argc)--; (index)++; }
int do_eeprom(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int layout_ver = LAYOUT_VERSION_AUTODETECT;
	enum eeprom_action action = EEPROM_ACTION_INVALID;
	int index = 0;
	ulong device_addr = -1, mem_addr = 0, cnt = 0, off = 0;
	int ret;
	char *field_name = "";
	char *field_value = "";

	if (argc <= 1)
		return CMD_RET_USAGE;

	NEXT_PARAM(argc, index);							/* Skip program name */

	action = parse_action(argv[index]);
	NEXT_PARAM(argc, index);
	if (action == EEPROM_ACTION_INVALID)
		return CMD_RET_USAGE;

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (action == EEPROM_PRINT || action == EEPROM_UPDATE) {
		if (!strcmp(argv[index], "-l")) {
			NEXT_PARAM(argc, index);
			layout_ver = eeprom_parse_layout_version(argv[index]);
			NEXT_PARAM(argc, index);
		}
	}
#endif

	if (action == EEPROM_READ || action == EEPROM_WRITE) {
		device_addr = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		mem_addr = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		off = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		cnt = parse_numeric_param(argv[index]);
	}

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (action == EEPROM_UPDATE) {
		device_addr = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		field_name = argv[index];
		NEXT_PARAM(argc, index);
		field_value = argv[index];
		NEXT_PARAM(argc, index);
	}
#endif

	return eeprom_execute_command(action, device_addr, layout_ver,
				      field_name, field_value, mem_addr, off, cnt);
}

U_BOOT_CMD(
	eeprom,	8,	1,	do_eeprom,
	"EEPROM sub-system (Parallel)",
	"read  <devaddr> addr off cnt\n"
	"eeprom write <devaddr> addr off cnt\n"
	"       - read/write `cnt` bytes from `devaddr` EEPROM, EEPROM offset `off` to/from memory `addr`"
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	"\n"
	"eeprom print [-l <layout_version>] <devaddr>\n"
	"       - Print layout fields and their data in human readable format\n"
	"eeprom update [-l <layout_version>] <devaddr> field_name field_value\n"
	"       - Update a specific eeprom field with new data.\n"
	"         The new data must be written in the same human readable format as shown by the print command.\n"
	"\n"
	"LAYOUT VERSIONS\n"
	"The -l option can be used to force the command to interpret the EEPROM data using the chosen layout.\n"
	"If the -l option is omitted, the command will auto detect the layout based on the data in the EEPROM.\n"
	"The values which can be provided with the -l option are:\n"
	CONFIG_EEPROM_LAYOUT_HELP_STRING"\n"
#endif
)
