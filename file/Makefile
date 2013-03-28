#
# Makefile for example
#

CC=cc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-g
ECHO=echo
TARGET=file
DFLAGS=-D_GNU_SOURCE -DDEBUG_ENCODING

SOURCES=$(wildcard *.c)
OBJECTS=$(subst .c,.o, $(SOURCES))

all: $(OBJECTS) $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) $< -o $@
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(TARGET)
