TARGET = m68kmon.c2500

all: $(TARGET)

# m68kMon
#################################################################
m68kmon.o: m68kmon.s
	m68k-linux-gnu-as -m68030 -a=m68kmon.lst -o m68kmon.o m68kmon.s cisco-2500/cisco-2500_lib.s

m68kmon.elf: m68kmon.o
	m68k-linux-gnu-ld -nostdlib -o m68kmon.elf m68kmon.o

m68kmon.bin: m68kmon.elf
	m68k-linux-gnu-objcopy -I elf32-m68k -O binary m68kmon.o m68kmon.bin

m68kmon.split: m68kmon.bin
	srec_cat m68kmon.bin -Binary -split 2 0 -o m68kmon.even.bin -Binary
	srec_cat m68kmon.bin -Binary -split 2 1 -o m68kmon.odd.bin -Binary

m68kmon.c2500: m68kmon.split
	python2.7 bin/flip_bits.py m68kmon.odd.bin m68kmon.c2500.fw1
	python2.7 bin/flip_bits.py m68kmon.even.bin m68kmon.c2500.fw2

clean:
	$(RM) m68kmon.lst m68kmon.o m68kmon.elf m68kmon.srec m68kmon.bin m68kmon.even.bin m68kmon.odd.bin m68kmon.c2500.fw1 m68kmon.c2500.fw2
