CFLAGS=-Wall -Werror -pedantic -march=native -O2 `sdl2-config --cflags`
LDFLAGS=`sdl2-config --libs`
BIN_NAME="chip8"

main: main.o chip8.o
	$(CC) $(LDFLAGS) -o $(BIN_NAME) *.o

all: main

clean:
	rm -f *.o
	rm -f $(BIN_NAME)