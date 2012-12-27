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

DDEBUG=-D__DEBUG__

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))

#TARGET=sync

all: $(OBJECTS)

#$(TARGET): $(OBJECTS)
#	$(CC) $(LDFLAGS) $(OBJECTS) -o $(BIN_DIR)/$@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $(DDEBUG) $< -o $@

clean:
	${RM} -f $(OBJECTS) $(BIN_DIR)/${TARGET}
