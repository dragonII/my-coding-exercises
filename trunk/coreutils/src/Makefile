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

yes:yes.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

sync:sync.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

logname: logname.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

unlink: unlink.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

whoami: whoami.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

link: link.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

dirname: dirname.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

tty: tty.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

#nproc not finished
#nproc: nproc.c
#	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)



$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) 
