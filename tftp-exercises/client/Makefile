#
# Makefile for example
#

OBJ_DIR=../obj
BIN_DIR=../bin

CC=cc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-g

DDEBUG=-D__DEBUG__

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))

TARGET=tftpclient

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(BIN_DIR)/$@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DDEBUG) $< -o $@

clean:
	${RM} -f $(OBJ_DIR)/$(OBJECTS) $(BIN_DIR)/${TARGET}
