TARGET = monitor-simple.o monitor-simple.run monitor-simple.bin monitor-simple.hex

all: $(TARGET)

monitor-simple.o: monitor-simple.s
	m68k-linux-gnu-as -m68000 -a=monitor-simple.lst -o monitor-simple.o monitor-simple.s

monitor-simple.run: monitor-simple.o
	m68k-linux-gnu-objcopy -I elf32-m68k -O srec monitor-simple.o monitor-simple.run

monitor-simple.bin: monitor-simple.o
	m68k-linux-gnu-objcopy -I elf32-m68k -O binary monitor-simple.o monitor-simple.bin

monitor-simple.hex: monitor-simple.run
	srec_cat monitor-simple.run -Motorola -output monitor-simple.hex -Intel

m68kmon.o: m68kmon.s
	m68k-linux-gnu-as -m68000 -a=m68kmon.lst -o m68kmon.o m68kmon.s cisco-2500/cisco-2500_lib.s

m68kmon.elf: m68kmon.o
	m68k-linux-gnu-ld -nostdlib -o m68kmon.elf m68kmon.o

m68kmon.bin: m68kmon.elf
	m68k-linux-gnu-objcopy -I elf32-m68k -O binary m68kmon.o m68kmon.bin

#m68kmon.run: m68kmon.o
#	m68k-linux-gnu-objcopy -I elf32-m68k -O srec m68kmon.o m68kmon.run
#	m68k-elf-objcopy -I elf32-m68k -O srec monitor.o temp
#	srec_cat temp -fill 0x00 0x8000 0xc000 --crop 0x8000 0xc000 -o monitor.run
#	srec_cat monitor.run -split 2 0 -offset 0x4000 -o monitor_even.run
#	srec_cat monitor.run -split 2 1 -offset 0x4000 -o monitor_odd.run
#	$(RM) temp

#monitor.hex: monitor.run
#	srec_cat monitor.run -Motorola -output monitor.hex -Intel

clean:
	$(RM) monitor-simple.o monitor-simple.run monitor-simple.bin monitor-simple.hex
	$(RM) m68kmon.lst m68kmon.o m68kmon.elf m68kmon.run m68kmon.bin
#	$(RM) monitor_even.run monitor_odd.run monitor.hex minimon.hex
