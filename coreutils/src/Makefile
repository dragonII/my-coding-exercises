#
# Makefile for example
#

OBJ_DIR=../obj
BIN_DIR=../bin

CC=cc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-g
INFLAGS=-I../lib

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))

all: $(OBJECTS)

sync:sync.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$@

logname: logname.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) 
