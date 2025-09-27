.PHONY: run

CCX := g++
CCXFLAGS += -g -Wall -pedantic

main: main.cpp $(wildcard *.hpp)
	$(CCX) main.cpp $(CCXFLAGS) -o main -std=c++26

run: main
	@ ./main

clean:
	rm -f main
