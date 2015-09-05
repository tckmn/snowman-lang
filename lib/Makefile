files := $(wildcard *.cpp *.hpp)

all: $(files)
	g++ $(files) -o snowman -std=c++11 -Wall -O0 -g

release: $(files)
	g++ $(files) -o snowman -std=c++11 -Wall -O3

afl:
	AFL_PATH=/home/llama/Programs/afl-1.83b afl-g++ main.cpp snowman.cpp snowman.hpp -o snowman -std=c++11 -Wall -O0 -g
