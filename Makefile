.PHONY:
	all

HEADERS = src/board.h src/piece.h
SOURCES = src/board.cc src/piece.cc src/xboard.cc

all: $(HEADERS) $(SOURCES)
	c++ -o bin/DeepBlur_debug -std=c++11 -Wall -g $(SOURCES)

o: $(HEADERS) $(SOURCES)
	c++ -o bin/DeepBlur -std=c++11 -O3 $(SOURCES)
