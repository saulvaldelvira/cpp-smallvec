.PHONY: run

CCX := g++

main: main.cpp $(wildcard *.hpp)
	$(CCX) main.cpp -g -o main -std=c++26

run: main
	@ ./main

clean:
	rm -f main
