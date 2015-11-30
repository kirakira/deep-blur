CFLAGS=-std=c++11 -Wall -Wextra -Wshadow -Werror
CC=c++
OUT=bin
ifeq ($(build),opt)
	CFLAGS+=-O3
else
	CFLAGS+=-g
endif

$(OUT)/common.o: src/common.h src/common.cc
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) -c -o $(OUT)/common.o src/common.cc

$(OUT)/board.o: $(OUT)/common.o src/board-common.h src/bitboard.h src/bitboard.cc
	$(CC) $(CFLAGS) -c -o $(OUT)/board.o src/bitboard.cc

$(OUT)/board_test: $(OUT)/board.o src/board_test.cc
	$(CC) $(CFLAGS) -o $(OUT)/board_test $(OUT)/board.o src/board_test.cc

.PHONY: test
test: $(OUT)/board_test

.PHONY: clean
clean:
	rm $(OUT)/*.o 2>/dev/null
