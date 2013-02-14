.PHONY:
	all

SOURCES = src/xiangqi/ai/Board.java src/xiangqi/ai/Piece.java src/xiangqi/Main.java src/xiangqi/ai/Agent.java src/xiangqi/ai/Move.java

all: $(SOURCES)
	javac -d bin -Xlint $(SOURCES)

jar:
	cd bin; jar -cfe xiangqi.jar xiangqi.Main xiangqi
