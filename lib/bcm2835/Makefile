CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -g

all: bcm_test
SOURCES=libnrf24.c bcm2835.c libnrf24-rpi.c
OBJ=$(SOURCES:.c=.o)

bcm_test: main.c $(OBJ)
	 gcc ${CCFLAGS} -Wall -I../ main.c $(OBJ) -o $@ 

%.o: %.c
	 gcc ${CCFLAGS} -Wall -I../ -o $@ -c $<

libnrf24.o: ../libnrf24.c
	gcc $(CCFLAGS) -Wall -I../ -o $@ -c $<
	
clean:
	rm $(OBJ)
	rm bcm_test
