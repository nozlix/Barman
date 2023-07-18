CC=gcc
CFLAGS= -g
LDFLAGS=-lpthread
LDLIBS=
RM=rm -fv
.PHONY: all clean
all: programme.out
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $< 
programme.out: programme.o main.o communication.o controle.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
clean:
	$(RM) *.o *.out