# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

## Build a couple of necessary functions into a private libgcc
## if the user asked for it
lib-$(CONFIG_USE_PRIVATE_LIBGCC) += lshrdi3.o muldi3.o ashldi3.o

obj-y	+= bdinfo.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y	+= cache.o
obj-y	+= interrupts.o
obj-y	+= traps.o

ifdef CONFIG_M680x0
obj-y	+= time_680x0.o
else
obj-y	+= time.o
obj-y	+= fec.o
endif
