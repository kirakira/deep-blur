.PHONY:
	all

SOURCES = src/xiangqi/ai/Board.java src/xiangqi/ai/Piece.java

all: $(SOURCES)
	javac -d bin $(SOURCES)
