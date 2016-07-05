CFLAGS=-I. -Wall

BIN=../bin/sac2sh
OBJ=sac2sh.o

sac2sh: $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)
	