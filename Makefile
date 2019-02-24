
CC=gcc
CFLAGS=-g 
CPPFLAGS=-Wall -Werror
SEMFLAG=-lpthread

PROGS=resend shell

.PHONY: all
all: $(PROGS)

resend: resend.o
shell: shell.o shell-utils.o

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $< $(SEMFLAG)  
%: %.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS) $(SEMFLAG)

.PHONY: clean
clean:
	$(RM) $(PROGS) *.o
