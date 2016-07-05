OBJ=parses.o load_stationdata.o sh2sac.o

BIN=../../bin/sh2sac
CC=gcc
CFLAGS=-I../include/


all: compile

compile: $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

clean: 
	rm -f $(OBJ) $(BIN)

