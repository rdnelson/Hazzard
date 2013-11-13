PROGRAM=main
SRC=libspi.c uart.c
OBJ=$(SRC:.c=.o)
.PHONY: all

all: main.hex

%.o: %.c
	avr-gcc -Os -DF_CPU=16000000UL -nostartfiles -mmcu=atmega328p -I../-o $@ -c $<

$(PROGRAM).hex: $(PROGRAM).elf
	avr-objcopy -O ihex -R .eeprom main main.hex

$(PROGRAM).elf: $(OBJ) libnrf24.o
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p  -I../ -nostartfiles libnrf24.o $(OBJ) $(PROGRAM).c -save-temps -o main
clean:
		rm $(OBJ)

libnrf24.o:../libnrf24.c
	avr-gcc -Os -DF_CPU=16000000UL -nostartfiles -I../ -mmcu=atmega328p -o $@ -c $<

flash: main.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 57600 -U flash:w:main.hex