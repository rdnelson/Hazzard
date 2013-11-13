CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

all: bcm_test
SOURCES=../libnrf24.c bcm2835.c
OBJ=$(SOURCES:.c=.o)

bcm_test: libnrf24-rpi.c $(OBJ)
	 gcc ${CCFLAGS} -Wall -I../ libnrf24-rpi.c $(OBJ) -o $@ 

%.o: %.c
	 gcc ${CCFLAGS} -Wall -I../ -o $@ -c $<
