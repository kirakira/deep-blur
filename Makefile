.PHONY:
	all

HEADERS = src/Board.h src/Piece.h
SOURCES = src/Board.cc src/Piece.cc src/xboard.cc

all: $(HEADERS) $(SOURCES)
	c++ -o bin/DeepBlur_debug -std=c++11 -Wall $(SOURCES)

o: $(HEADERS) $(SOURCES)
	c++ -o bin/DeepBlur -std=c++11 -O3 $(SOURCES)
