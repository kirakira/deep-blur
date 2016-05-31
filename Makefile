COMPILER=c++
COMPILER_FLAGS=-std=c++14 -Wall -Wextra -Wshadow -Werror -fconstexpr-steps=100000000
OUT=bin
ifeq ($(build),opt)
	COMPILER_FLAGS+=-O3 -march=native
else
	COMPILER_FLAGS+=-g
endif

.PHONY: test
test: $(OUT)/common_test $(OUT)/bittables_test $(OUT)/board_test

$(OUT)/common.o: src/common.h src/common.cc
	mkdir -p $(OUT)
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/common.cc

$(OUT)/bittables.o: $(OUT)/common.o src/board-base.h src/bitboard.h src/bittables.h src/bittables.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/bittables.cc

$(OUT)/board.o: $(OUT)/common.o $(OUT)/bittables.o src/board.h src/bitboard.h src/board-base.h src/board.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/board.cc

$(OUT)/common_test: $(OUT)/common.o src/common_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o src/common_test.cc

$(OUT)/bittables_test: $(OUT)/bittables.o $(OUT)/common.o src/bittables_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/bittables.o $(OUT)/common.o src/bittables_test.cc

$(OUT)/board_test: $(OUT)/board.o $(OUT)/bittables.o $(OUT)/common.o src/board_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/board.o $(OUT)/bittables.o $(OUT)/common.o src/board_test.cc

.PHONY: clean
clean:
	rm -f $(OUT)/*.o $(OUT)/*_test
