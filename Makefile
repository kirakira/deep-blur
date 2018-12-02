COMPILER=clang++
COMPILER_FLAGS=-std=c++14 -Wall -Wextra -Wshadow -Werror -Wconversion -Wno-sign-conversion -fconstexpr-steps=10000000
OUT=bin
ifeq ($(build),dbg)
	COMPILER_FLAGS+=-g
else
	COMPILER_FLAGS+=-O3 -march=native -flto -DNDEBUG
endif

.PHONY: test
test: $(OUT)/common_test $(OUT)/bittables_test $(OUT)/bitboard_test $(OUT)/board_test $(OUT)/genmove_test $(OUT)/transposition_test $(OUT)/see_test

$(OUT)/common.o: src/common.h src/common.cc
	mkdir -p $(OUT)
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/common.cc

$(OUT)/board-base.o: src/common.h src/board-base.h src/board-base.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/board-base.cc

$(OUT)/board-hash.o: src/common.h src/board-base.h src/board-hash.h src/board-hash.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/board-hash.cc

$(OUT)/eval.o: src/board-base.h src/eval.h src/piece-value-eval.h src/piece-position-eval.h src/common.h src/eval.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/eval.cc

$(OUT)/piece-position-eval.o: src/board-base.h src/eval.h src/piece-position-eval.h src/piece-position-eval.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/piece-position-eval.cc

$(OUT)/piece-value-eval.o: src/board-base.h src/eval.h src/piece-value-eval.h src/piece-value-eval.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/piece-value-eval.cc

$(OUT)/logger.o: src/logger.h src/logger.cc src/common.h src/board-base.h
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/logger.cc

$(OUT)/board.o: src/common.h src/eval.h src/bittables.h src/board.h src/bitboard.h src/board-base.h src/board-hash.h src/board.cc
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/board.cc

$(OUT)/transposition.o: src/transposition.h src/transposition.cc src/board-base.h src/common.h
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/transposition.cc

$(OUT)/move-picker.o: src/move-picker.h src/move-picker.cc src/board-base.h src/board.h src/common.h src/see.h
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/move-picker.cc

$(OUT)/see.o: src/see.h src/see.cc src/board.h src/board-base.h
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/see.cc

$(OUT)/search.o: src/search.h src/search.cc src/board.h src/common.h src/eval.h src/piece-value-eval.h src/bittables.h src/bitboard.h src/board-base.h src/board-hash.h src/logger.h src/transposition.h src/move-picker.h
	$(COMPILER) $(COMPILER_FLAGS) -c -o $@ src/search.cc

$(OUT)/xboard: $(OUT)/common.o $(OUT)/search.o $(OUT)/logger.o $(OUT)/board.o $(OUT)/board-hash.o $(OUT)/board-base.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/search.o $(OUT)/transposition.o $(OUT)/move-picker.o $(OUT)/see.o src/xboard.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ src/xboard.cc $(OUT)/common.o $(OUT)/search.o $(OUT)/logger.o $(OUT)/board.o $(OUT)/board-hash.o $(OUT)/board-base.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/transposition.o $(OUT)/move-picker.o $(OUT)/see.o

$(OUT)/common_test: $(OUT)/common.o src/common_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o src/common_test.cc

$(OUT)/bitboard_test: $(OUT)/common.o $(OUT)/board-base.o src/bitboard.h src/bitboard_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o $(OUT)/board-base.o src/bitboard_test.cc

$(OUT)/bittables_test: $(OUT)/common.o src/bittables.h $(OUT)/board-base.o src/bittables_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o $(OUT)/board-base.o src/bittables_test.cc

$(OUT)/board_test: $(OUT)/common.o $(OUT)/board.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/board-hash.o $(OUT)/board-base.o src/board_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/board.o $(OUT)/eval.o $(OUT)/piece-value-eval.o $(OUT)/piece-position-eval.o $(OUT)/board-hash.o $(OUT)/common.o $(OUT)/board-base.o src/board_test.cc

$(OUT)/genmove_test: $(OUT)/common.o $(OUT)/board.o $(OUT)/eval.o $(OUT)/piece-value-eval.o $(OUT)/piece-position-eval.o $(OUT)/board-hash.o $(OUT)/board-base.o src/genmove_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/board.o $(OUT)/eval.o $(OUT)/piece-value-eval.o $(OUT)/piece-position-eval.o $(OUT)/board-hash.o $(OUT)/common.o $(OUT)/board-base.o src/genmove_test.cc

$(OUT)/transposition_test: $(OUT)/common.o $(OUT)/board-base.o $(OUT)/transposition.o src/transposition_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o $(OUT)/board-base.o $(OUT)/transposition.o src/transposition_test.cc

$(OUT)/see_test: $(OUT)/common.o $(OUT)/board.o $(OUT)/board-base.o $(OUT)/board-hash.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/see.o src/see_test.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/common.o $(OUT)/board.o $(OUT)/board-base.o $(OUT)/board-hash.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/see.o src/see_test.cc

$(OUT)/board_benchmark: $(OUT)/common.o $(OUT)/board.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/board-hash.o $(OUT)/board-base.o src/board_benchmark.cc
	$(COMPILER) $(COMPILER_FLAGS) -o $@ $(OUT)/board.o $(OUT)/eval.o $(OUT)/piece-position-eval.o $(OUT)/piece-value-eval.o $(OUT)/board-hash.o $(OUT)/common.o $(OUT)/board-base.o src/board_benchmark.cc

.PHONY: clean
clean:
	rm -f $(OUT)/*.o $(OUT)/*_test $(OUT)/xboard $(OUT)/board_benchmark
