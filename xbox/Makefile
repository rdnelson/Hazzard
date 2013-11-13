CPP:=gcc -Wall
APPSRC:=xbox.c main.c
APPOBJ:=$(APPSRC:.c=.o)
LIBSRC:=xbox.c
LIBOBJ:=$(LIBSRC:.c=.lo)

.PHONY: all clean

all: xbox_test xbox.so

clean:
	$(RM) $(APPOBJ) $(LIBOBJ) xbox_test xbox.so

xbox_test: $(APPOBJ)
	$(CPP) -g -o $@ $^

xbox.so: $(LIBOBJ)
	$(CPP) -g -shared -fPIC -o $@ $^

%.o: %.c
	$(CPP) -c -g -o $@ $^

%.lo: %.c
	$(CPP) -c -g -fPIC -shared -o $@ $^
