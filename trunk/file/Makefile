#
# Makefile for example
#

CC=cc
RM=rm
CFLAGS=-Wall -c -g
LDFLAGS=-g
INFLAGS=-I../lib
ECHO=echo
TARGET=file

SOURCES=$(wildcard *.c)
#OBJECTS=$(addprefix $(OBJ_DIR)/,$(subst .c,.o, $(SOURCES)))
OBJECTS=$(subst .c,.o, $(SOURCES))

all: $(OBJECTS) $(TARGET)

%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@
$(TARGET): $(SOURCES)
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(TARGET)
