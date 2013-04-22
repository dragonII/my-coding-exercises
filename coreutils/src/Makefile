#
# Makefile for example
#

OBJ_DIR=../obj
BIN_DIR=../bin

CC=cc
RM=rm
CFLAGS=-Wall -c -g -D_GNU_SOURCE
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
env: env.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
readlink: readlink.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
tee: tee.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
nice: nice.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
setuidgid: setuidgid.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
mkdir: mkdir.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
chroot: chroot.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
sum: sum.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
rmdir: rmdir.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
mknod: mknod.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
echo: echo.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
nohup: nohup.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
runcon: runcon.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
fold: fold.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
chgrp: chgrp.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
id: id.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
kill: kill.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
cksum: cksum.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
#uname: uname.c
#	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
chown: chown.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
mktemp: mktemp.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
rm: rm.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
pwd: pwd.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
seq: seq.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
comm: comm.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)
dircolors: dircolors.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $(OBJ_DIR)/$(addsuffix .o,$@)





$(OBJ_DIR)/%.o: %.c
	$(CC) $(INFLAGS) $(CFLAGS) $< -o $@


clean:
	${RM} -f $(OBJECTS) $(OBJ_DIR_BINARIES)
