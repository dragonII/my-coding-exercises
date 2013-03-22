#
# Makefile for example
#

CC=cc
RM=rm
CFLAGS=-Wall -c -g -D_GNU_SOURCE
LDFLAGS=-g
INFLAGS=-I../lib

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))
OBJECTS=$(subst .c,.o, $(SOURCES))
OBJ_DIR_BINARIES=$(addprefix $(OBJ_DIR)/,$(subst .c,, $(SOURCES)))

all: $(OBJECTS)

%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(OBJ_DIR_BINARIES)
