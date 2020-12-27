TARGET = divide_by_zero.srec trap0.srec

all: $(TARGET)

# Divide by zero exception
#################################################################
divide_by_zero.srec: divide_by_zero.s
	m68k-linux-gnu-as -m68000 -o divide_by_zero.o divide_by_zero.s
	m68k-linux-gnu-objcopy -I elf32-m68k --srec-forceS3 -O srec divide_by_zero.o divide_by_zero.tmp
	srec_cat divide_by_zero.tmp -offset 0x1000 -output divide_by_zero.srec -Motorola -Address_Length 4

# TRAP0
#################################################################
trap0.srec: trap0.s
	m68k-linux-gnu-as -m68000 -o trap0.o trap0.s
	m68k-linux-gnu-objcopy -I elf32-m68k --srec-forceS3 -O srec trap0.o trap0.tmp
	srec_cat trap0.tmp -offset 0x1000 -output trap0.srec -Motorola -Address_Length 4

# Clean
#################################################################
clean:
	$(RM) divide_by_zero.o divide_by_zero.tmp divide_by_zero.srec
	$(RM) trap0.o trap0.tmp trap0.srec
