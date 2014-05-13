
OBJ_DIR=../../obj
LIB_DIR=../../lib
BIN_DIR=../../bin

CC=cc
RM=rm
#CFLAGS=-Wall -c -g -D_GNU_SOURCE
CFLAGS=-Wall -c -fPIC
LDFLAGS=-shared
INFLAGS=-I. -I..
DEBUG=-g

TARGET=libptrx_file.so

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))
OBJ_DIR_BINARIES=$(addprefix $(OBJ_DIR)/,$(subst .c,, $(SOURCES)))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(LIB_DIR)/$(TARGET) $(OBJECTS)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@

clean:
	${RM} -f $(OBJECTS)
	${RM} -f $(LIB_DIR)/$(TARGET)
