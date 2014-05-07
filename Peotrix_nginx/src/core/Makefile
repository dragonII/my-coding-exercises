
OBJ_DIR=../../obj
BIN_DIR=../../bin

CC=cc
RM=rm
#CFLAGS=-Wall -c -g -D_GNU_SOURCE
CFLAGS=-Wall -c
LDFLAGS=
INFLAGS=-I.
DEBUG=-g

TARGET=PeoTrix

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))
OBJ_DIR_BINARIES=$(addprefix $(OBJ_DIR)/,$(subst .c,, $(SOURCES)))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$(TARGET) $(OBJECTS)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@

clean:
	${RM} -f $(OBJ_DIR)/*
	${RM} -f $(BIN_DIR)/*
