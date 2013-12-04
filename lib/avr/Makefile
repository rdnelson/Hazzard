PROGRAMS=main
SRC=libspi.c uart.c
TARGETS=$(PROGRAMS:%=%.hex)
OBJ=$(SRC:.c=.o)
.PHONY: all

all: $(TARGETS)

%.o: %.c
	avr-gcc -Os -DF_CPU=8000000UL -nostartfiles -mmcu=atmega328p -I../ -o $@ -c $<

%.hex: %.elf
	avr-objcopy --strip-all -O ihex -R .eeprom $< $@

%.elf: libnrf24.o %.c $(OBJ) 
	avr-gcc -Os -DF_CPU=8000000UL -mmcu=atmega328p  -I../ $^  -o $@


clean:
		rm $(OBJ)
		rm libnrf24.o
		rm main
		rm $(TARGETS)

libnrf24.o:../libnrf24.c
	avr-gcc -Os -DF_CPU=8000000UL -nostartfiles -I../ -mmcu=atmega328p -o $@ -c $<



flash: main.hex
	avrdude -F -V -c usbtiny -p ATMEGA328P  -U flash:w:main.hex
