#
# Makefile for example
#

CC=cc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-g
ECHO=echo
TARGET=file
DFLAGS=-D_ISOC99_SOURCE -D_GNU_SOURCE -DDEBUG_ENCODING
LFLAGS=-lz

SOURCES=$(wildcard *.c)
OBJECTS=$(subst .c,.o, $(SOURCES))

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) $< -o $@
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@


clean:
	${RM} -f $(OBJECTS) $(TARGET)
