.PHONY:
	all

HEADERS = src/board.h src/piece.h src/move.h src/rc4.h src/agent.h src/transposition.h src/see.h src/movelist.h src/common.h src/hash.h
SOURCES = src/board.cc src/agent.cc src/xboard.cc src/transposition.cc src/movelist.cc src/see.cc src/common.cc src/hash.cc

all: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur_debug -Wall -Wextra -g $(SOURCES)

o: $(HEADERS) $(SOURCES)
	g++ -o bin/deep-blur -O3 $(SOURCES)
