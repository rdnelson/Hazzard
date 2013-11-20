PROGRAMS=main main3
SRC=libspi.c uart.c
TARGETS=$(PROGRAMS:%=%.hex)
OBJ=$(SRC:.c=.o)
.PHONY: all

all: $(TARGETS)

%.o: %.c
	avr-gcc -Os -DF_CPU=16000000UL -nostartfiles -mmcu=atmega328p -I../ -o $@ -c $<

%.hex: %.elf
	avr-objcopy --strip-all -O ihex -R .eeprom $< $@

%.elf: libnrf24.o %.c $(OBJ) 
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p  -I../ -nostartfiles $^  -o $@


clean:
		rm $(OBJ)
		rm libnrf24.o
		rm main
		rm $(TARGETS)

libnrf24.o:../libnrf24.c
	avr-gcc -Os -DF_CPU=16000000UL -nostartfiles -I../ -mmcu=atmega328p -o $@ -c $<



flash: main.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 57600 -U flash:w:main.hex