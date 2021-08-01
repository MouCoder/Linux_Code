BIN=HttpServer
SRC=main.cc
CC=g++
FLAGS=-std=c++11 -lpthread #-DDEBUG

$(BIN):$(SRC)
	$(CC) -o $@ $^ $(FLAGS)
.PHONY:clean
clean:
	rm -f $(BIN)

