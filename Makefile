.PHONY:
	all

SOURCES = src/xiangqi/ai/Board.java src/xiangqi/ai/Piece.java src/xiangqi/Main.java

all: $(SOURCES)
	javac -d bin -Xlint $(SOURCES)
