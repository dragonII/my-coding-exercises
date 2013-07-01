
CC	= gcc
CFLAGS	= -Wall -O2 -fomit-frame-pointer -D_GNU_SOURCE
#LDFLAGS	= -s
STATIC	=

# For some known distributions we do not build all programs, otherwise we do.
BIN	=
SBIN	= init halt shutdown runlevel killall5
USRBIN	= last mesg

BIN_OWNER	= root
BIN_GROUP	= root
BIN_COMBO	= $(BIN_OWNER):$(BIN_GROUP)
INSTALL		= install -o $(BIN_OWNER) -g $(BIN_GROUP)
MANDIR		= /usr/share/man


wall:		dowall.o wall.o
		$(CC) $(LDFLAGS) -o dowall.o wall.o

shutdown:	dowall.o shutdown.o utmp.o reboot.h
		$(CC) $(LDFLAGS) -o $@ dowall.o shutdown.o utmp.o

dowall.o:	dowall.c 
		$(CC) -c $(CFLAGS) dowall.c


clean:		
		rm -f *.o

