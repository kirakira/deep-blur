#include <cassert>
#include <iostream>
#include <map>

#include "board.h"

using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

namespace blur {

Board::Board() {
  assert(
      SetBoard("rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR"));
}

bool Board::SetBoard(const string& fen) {
  vector<Side> sides = {Side::kRed, Side::kBlack};
  vector<PieceType> piece_types = {PieceType::kKing,      PieceType::kPawn,
                                   PieceType::kAssistant, PieceType::kElephant,
                                   PieceType::kHorse,     PieceType::kCannon,
                                   PieceType::kRook};
  map<char, Piece> char_piece_map;
  for (auto side : sides) {
    for (auto piece_type : piece_types) {
      Piece piece(side, piece_type);
      char_piece_map[piece.ToLetter()] = piece;
    }
  }

  int row = 9, col = 0;
  bool rk_found = false, bk_found = false;
  for (char c : fen) {
    if (c >= '0' && c <= '9') {
      col += (c - '0');
    } else if (c == '/') {
      if (col != kNumColumns) return false;
      --row;
      col = 0;
      if (row < 0) return false;
    } else {
      if (char_piece_map.count(c) == 0) return false;
      if (char_piece_map[c] == Piece(Side::kRed, PieceType::kKing)) {
        if (!(row <= 2 && col >= 3 && col <= 5)) return false;
        rk_found = true;
      } else if (char_piece_map[c] == Piece(Side::kBlack, PieceType::kKing)) {
        if (!(row >= 7 && col >= 3 && col <= 5)) return false;
        bk_found = true;
      }
      if (col >= kNumColumns) return false;
      ++col;
    }
  }
  if (row != 0) return false;
  if (col != kNumColumns) return false;
  if (!rk_found || !bk_found) return false;

  for (int i = 0; i < kNumRows; ++i) {
    for (int j = 0; j < kNumColumns; ++j) {
      board_[i][j] = Piece::EmptyPiece();
    }
  }
  for (int i = 0; i < 16; ++i) {
    piece_bitboards_[i] = BitBoard::EmptyBoard();
  }

  row = 9, col = 0;
  for (char c : fen) {
    if (c >= '0' && c <= '9') {
      col += (c - '0');
    } else if (c == '/') {
      --row;
      col = 0;
    } else {
      Piece piece = char_piece_map[c];
      board_[row][col] = piece;
      piece_bitboards_[piece.value()] |= BitBoard::Fill(Position(row, col));

      ++col;
    }
  }

  return true;
}

string Board::ToString() const {
  string ans;
  int gap = 0;
  const auto append_gap = [&ans, &gap]() {
    if (gap > 0) {
      ans += std::to_string(gap);
      gap = 0;
    }
  };
  for (int i = kNumRows - 1; i >= 0; --i) {
    for (int j = 0;j < kNumColumns; ++j){ 
      if (board_[i][j] != Piece::EmptyPiece()) {
        append_gap();
        ans += board_[i][j].ToLetter();
      } else {
        ++gap;
      }
    }
    append_gap();
    if (i != 0) ans += "/";
  }
  return ans;
}

}  // namespace blur
