files := $(wildcard *.cpp *.hpp)

all: $(files)
	g++ $(files) -o snowman -std=c++11 -Wall -O0 -g

release: $(files)
	g++ $(files) -o snowman -std=c++11 -Wall -O3
