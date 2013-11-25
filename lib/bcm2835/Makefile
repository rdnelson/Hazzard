CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -g
INCL_DIR=../
LFLAGS= -lncurses


CCFLAGS+=-I $(INCL_DIR)
PROGS=bcm_test.c drive_test.c

TARGETS= $(patsubst %.c,%.bin,$(PROGS))

SOURCES=libnrf24.c bcm2835.c libnrf24-rpi.c
OBJ=$(SOURCES:.c=.o)
.SECONDARY: $(OBJS)
vpath %.c ../

.PHONY: all

all:  $(TARGETS)

%.bin: %.c $(OBJ)
	 gcc ${CCFLAGS} -Wall $(LFLAGS) $^ -o $(patsubst %.bin, %, $@) 

%.o: %.c
	 gcc ${CCFLAGS} -Wall -o $@ -c $<

clean:
	rm $(OBJ)
	rm bcm_test
	
OLD_SHELL := $(SHELL)
SHELL=$(warning [$@ ($^) ($?)])$(OLD_SHELL)
