.PHONY:
	all

HEADERS = src/board.h src/piece.h src/move.h src/rc4.h src/agent.h src/transposition.h
SOURCES = src/board.cc src/agent.cc src/xboard.cc src/transposition.cc

all: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur_debug -Wall -g $(SOURCES)

o: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur -O3 $(SOURCES)
