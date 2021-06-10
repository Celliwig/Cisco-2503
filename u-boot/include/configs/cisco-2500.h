/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Cisco 2500 board configuration
 *
 * (C) Copyright 2021 Celliwig
 */

/*
 * Memory layout
 *---------------
 * 0x0000-0x03FF = Interrupt vector table
 * 0x0400-0x0FFF = Early static variables/stack/global data area/malloc
 */

#ifndef __CISCO2500_CONFIG_H
#define __CISCO2500_CONFIG_H

#define CONFIG_HOSTNAME				"cisco-2500"

#define CONFIG_SYS_BAUDRATE_TABLE		{ 1200, 2400, 4800, 9600, 19200, 38400 }

//#define CONFIG_BOOTCOMMAND			"bootm ffc20000"
//#define CONFIG_EXTRA_ENV_SETTINGS				\
//	"upgrade_uboot=loady; "					\
//		"protect off 0xffc00000 0xffc1ffff; "		\
//		"erase 0xffc00000 0xffc1ffff; "			\
//		"cp.b 0x20000 0xffc00000 ${filesize}\0"		\
//	"upgrade_kernel=loady; "				\
//		"erase 0xffc20000 0xffefffff; "			\
//		"cp.b 0x20000 0xffc20000 ${filesize}\0"		\
//	"upgrade_jffs2=loady; "					\
//		"erase 0xfff00000 0xffffffff; "			\
//		"cp.b 0x20000 0xfff00000 ${filesize}\0"

/* undef to save memory	*/

#define CONFIG_SYS_LOAD_ADDR			0x2000	/* default load address */

#define CONFIG_SYS_HZ				1000
#define CONFIG_SYS_TMRINTR_NO			EVA_INTERRUPT_LEVEL7

#define CONFIG_SYS_CLK				25000000
#define CONFIG_SYS_CPU_CLK			CONFIG_SYS_CLK
/* Definitions for initial stack pointer and data area */
#define CONFIG_SYS_INIT_RAM_ADDR		0x0000400
/* size of initial stack (minus space for early static variables) */
#define CONFIG_SYS_INIT_RAM_SIZE		0xC00
//#define CONFIG_SYS_GBL_DATA_OFFSET		(CONFIG_SYS_INIT_RAM_SIZE - \
//						 GENERATED_GBL_DATA_SIZE)
//#define CONFIG_SYS_INIT_SP_OFFSET		CONFIG_SYS_GBL_DATA_OFFSET

#define	CONFIG_SYS_SDRAM_BASE			0x00000000
#define	CONFIG_SYS_SDRAM_SIZE			0x01000000		/* Max RAM size */
#define	CONFIG_SYS_FLASH_BASE			0x01000000
#define	CONFIG_SYS_MAX_FLASH_BANKS		1
#define	CONFIG_SYS_MAX_FLASH_SECT		1024
#define	CONFIG_SYS_FLASH_ERASE_TOUT		1000
#define	CONFIG_FLASH_CFI_LEGACY						/* Need to define flash part as it's unrecognisable */
#define	CONFIG_SYS_FLASH_QUIRK_DATA_BUS_REVERSED
#define	CONFIG_FLASH_VERIFY

//* reserve 128-2KB */
#define CONFIG_SYS_MONITOR_BASE			(CONFIG_SYS_FLASH_BASE + 0x400)
#define CONFIG_SYS_MONITOR_LEN			((256 - 2) * 1024)
//#define CONFIG_SYS_MALLOC_LEN			(1 * 1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN			(256 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN		(64 * 1024)

//#define LDS_BOARD_TEXT \
//	. = DEFINED(env_offset) ? env_offset : .; \
//	env/embedded.o(.text*);

///* memory map space for linux boot data */
//#define CONFIG_SYS_BOOTMAPSZ			(8 << 20)

/*
 * Cache Configuration
 *
 * Special 8K version 3 core cache.
 * This is a single unified instruction/data cache.
 * sdram - single region - no masks
 */
//#define CONFIG_SYS_CACHELINE_SIZE		16

#define	ICACHE_STATUS				CONFIG_SYS_INIT_RAM_ADDR
#define	DCACHE_STATUS				(ICACHE_STATUS + 4)

#define	CONFIG_SYS_CACHE_ICACR			0x0001
#define	CONFIG_SYS_ICACHE_INV			0x0008
#define	CONFIG_SYS_CACHE_DCACR			0x0100
#define	CONFIG_SYS_DCACHE_INV			0x0800

/* Environment EEPROM */
#define	CONFIG_SYS_DEF_EEPROM_ADDR		0x02000000
#define	CONFIG_SYS_EEPROM_SIZE			(32 * 1024)
#define	CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	7
#define	CONFIG_SYS_EEPROM_LAST_WRITE_TEST

///* CS0 - AMD Flash, address 0xffc00000 */
//#define	CONFIG_SYS_CS0_BASE		(CONFIG_SYS_FLASH_BASE>>16)
///* 4MB, AA=0,V=1  C/I BIT for errata */
//#define	CONFIG_SYS_CS0_MASK		0x003f0001
///* WS=10, AA=1, PS=16bit (10) */
//#define	CONFIG_SYS_CS0_CTRL		0x1980
///* CS1 - DM9000 Ethernet Controller, address 0x30000000 */
//#define CONFIG_SYS_CS1_BASE			0x3000
//#define CONFIG_SYS_CS1_MASK			0x00070001
//#define CONFIG_SYS_CS1_CTRL			0x0100

#endif  /* __CISCO2500_CONFIG_H */

