#
# Makefile for example
#

OBJ_DIR=../obj
BIN_DIR=../bin

CC=cc
RM=rm
CFLAGS=-Wall -c -g -D_GNU_SOURCE
LDFLAGS=-g
#INFLAGS=-I../lib

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))
OBJ_DIR_BINARIES=$(addprefix $(OBJ_DIR)/,$(subst .c,, $(SOURCES)))
BIN_DIR_BINARIES=$(addprefix $(BIN_DIR)/,$(subst .c,, $(SOURCES)))

#all: $(OBJECTS)

size:size.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)


$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(OBJ_DIR_BINARIES) $(BIN_DIR_BINARIES)
