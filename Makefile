.PHONY:
	all

HEADERS = src/board.h src/piece.h
SOURCES = src/board.cc src/piece.cc src/xboard.cc

all: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur_debug -std=c++11 -Wall -g $(SOURCES)

o: $(HEADERS) $(SOURCES)
	g++ -o bin/deepblur -std=c++11 -O3 $(SOURCES)
