#
# Makefile for example
#

CC=cc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-g
#INFLAGS=-I../lib
ECHO=echo
TARGET=file
DFLAGS=-D_GNU_SOURCE

SOURCES=$(wildcard *.c)
#OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))
OBJECTS=$(subst .c,.o, $(SOURCES))

all: $(OBJECTS) $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) $< -o $@
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(TARGET)
