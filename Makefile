.PHONY:
	all

HEADERS = src/board.h src/piece.h src/move.h src/rc4.h
SOURCES = src/board.cc src/piece.cc src/move.cc src/xboard.cc

all: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur_debug -Wall -g $(SOURCES)

o: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur -O3 $(SOURCES)
