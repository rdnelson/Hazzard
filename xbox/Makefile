CPP:=gcc -Wall
APPSRC:=xbox.c main.c
APPOBJ:=$(APPSRC:.c=.o)

xbox_app: $(APPOBJ)
	$(CPP) -g -o $@ $^

%.o: %.c
	$(CPP) -c -g -o $@ $^
