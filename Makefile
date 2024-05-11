CC=g++
CFLAGS=-I./include/ -Wall -std=c++20 -pedantic -O2 -g
LFLAGS=
EXE=main

${EXE}: build/main.o build/interpreter.o
	${CC} build/main.o build/interpreter.o -o build/${EXE}

build/main.o: src/main.cpp
	${CC} ${CFLAGS} -c src/main.cpp -o build/main.o

build/interpreter.o: src/interpreter.cpp include/interpreter.hpp
	${CC} ${CFLAGS} -c src/interpreter.cpp -o build/interpreter.o

.PHONY: clean
clean:
	rm -rf build/*
