COMPILER=c++
COMPILER_FLAGS=-std=c++14 -Wall -Wextra -Wshadow -Werror
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

$(OUT)/bitboard.o: $(OUT)/common.o src/board-common.h src/bitboard.h src/bitboard.cc src/bitboard-impl.h
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/bitboard.cc

$(OUT)/common_test: $(OUT)/common.o src/common_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o src/common_test.cc

$(OUT)/board_test: $(OUT)/bitboard.o $(OUT)/common.o src/board_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/bitboard.o $(OUT)/common.o src/board_test.cc

.PHONY: clean
clean:
	rm -f $(OUT)/*.o $(OUT)/*_test
