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
OBJ_DIR_BINARIES=$(addprefix $(OBJ_DIR)/,$(subst .c,, $(SOURCES)))

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

hostname: hostname.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

groups: groups.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

users: users.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

basename: basename.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)

# need link libselinux.so for executable binary
mkfifo: mkfifo.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
sleep: sleep.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
printenv: printenv.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)


$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(OBJ_DIR_BINARIES)
