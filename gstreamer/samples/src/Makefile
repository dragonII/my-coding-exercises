#
# Makefile for example
#

OBJ_DIR=../obj
BIN_DIR=../bin

CC=gcc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-L/usr/local/lib -lgstreamer-1.0
INFLAGS=-I/usr/local/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))

#TARGET=initializing

all: $(OBJECTS)

#initializing: initializing.o
#	$(CC) $(OBJECTS) $(LDFLAGS) -o $(BIN_DIR)/$@

#$(TARGET): $(OBJECTS)
#	$(CC) $(OBJECTS) $(LDFLAGS) -o $(BIN_DIR)/$@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INFLAGS) $< -o $@

clean:
	${RM} -f $(OBJ_DIR)/$(OBJECTS) $(BIN_DIR)/*
