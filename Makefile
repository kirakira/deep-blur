COMPILER=c++
COMPILER_FLAGS=-std=c++14 -Wall -Wextra -Wshadow -Werror -fconstexpr-steps=100000000
OUT=bin
ifeq ($(build),opt)
	COMPILER_FLAGS+=-O3
else
	COMPILER_FLAGS+=-g
endif

.PHONY: test
test: $(OUT)/common_test $(OUT)/board_test

$(OUT)/common.o: src/common.h src/common.cc
	mkdir -p $(OUT)
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/common.cc

$(OUT)/bitboard.o: $(OUT)/common.o src/board.h src/bitboard.h src/bitboard.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/bitboard.cc

$(OUT)/board.o: $(OUT)/common.o $(OUT)/bitboard.o src/board.h src/board.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/board.cc

$(OUT)/common_test: $(OUT)/common.o src/common_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o src/common_test.cc

$(OUT)/bitboard_test: $(OUT)/bitboard.o $(OUT)/common.o src/board_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/bitboard.o $(OUT)/common.o src/board_test.cc

.PHONY: clean
clean:
	rm -f $(OUT)/*.o $(OUT)/*_test
