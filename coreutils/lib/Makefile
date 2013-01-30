#
# Makefile for example
#

OBJ_DIR=../obj
BIN_DIR=../bin

CC=cc
RM=rm
CFLAGS=-Wall -c -g -D_GNU_SOURCE
LDFLAGS=-g
INFLAGS=-I../lib

#DDEBUG=-D__DEBUG__

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))

all: $(OBJECTS)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $(DDEBUG) $< -o $@

clean:
	${RM} -f $(OBJECTS)
