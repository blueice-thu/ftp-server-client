.PHONY:clean

CC = gcc
CFLAGS=-Wall -g
BIN = server
OBJS = main.o strutils.o netutils.o common.o command_access.o command_file.o command_mode.o command_trans.o
LIB = -lpthread

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
