all:
	g++ main.cpp snowman.cpp snowman.hpp -o snowman -std=c++11 -Wall -O0 -g

release:
	g++ main.cpp snowman.cpp snowman.hpp -o snowman -std=c++11 -Wall -O3

afl:
	AFL_PATH=/home/llama/Programs/afl-1.83b afl-g++ main.cpp snowman.cpp snowman.hpp -o snowman -std=c++11 -Wall -O0 -g
