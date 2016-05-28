#include <cassert>
#include <map>

#include "board.h"

using std::map;
using std::string;
using std::vector;

Board::Board() {
  assert(
      SetBoard("rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR"));
}

bool Board::SetBoard(const string& fen) {
  vector<Side> sides = {kRed, kBlack};
  vector<PieceType> piece_types = {kKing,  kPawn,   kAssistant, kElephant,
                                   kHorse, kCannon, kRook};
  map<char, Piece> char_piece_map;
  for (auto side : sides) {
    for (auto piece_type : piece_types) {
      Piece piece(side, piece_type);
      char_piece_map[piece.ToLetter()] = piece;
    }
  }

  int row = 0, col = 0;
  bool rk_found = false, bk_found = false;
  for (char c : fen) {
    if (c >= '0' && c <= '9') {
      col += (c - '0');
      if (col >= kNumColumns) return false;
    } else if (c == '/') {
      if (col != kNumColumns) return false;
      ++row;
      if (row >= kNumRows) return false;
    } else {
      if (char_piece_map.count(c) == 0) return false;
      if (char_piece_map[c] == Piece(Side::kRed, PieceType::kKing)) {
        if (!(row <= 2 && col >= 3 && col <= 5)) return false;
        rk_found = true;
      } else if (char_piece_map[c] == Piece(Side::kBlack, PieceType::kKing)) {
        if (!(row >= 7 && col >= 3 && col <= 5)) return false;
        bk_found = true;
      }
      ++col;
    }
  }
  if (row != kNumRows - 1) return false;
  if (col != kNumColumns) return false;
  if (!rk_found || !bk_found) return false;

  for (int i = 0; i < kNumRows; ++i) {
    for (int j = 0; j < kNumColumns; ++j) {
      board_[i][j] = Piece::Empty();
    }
  }
  for (int i = 0; i < 16; ++i) {
    piece_bitboards_[i] = BitBoard::EmtpyBoard();
  }

  row = 0, col = 0;
  for (char c : fen) {
    if (c >= '0' && c <= '9') {
      col += (c - '0');
    } else if (c == '/') {
      ++row;
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
  const auto append_gap[&ans, &gap]() {
    if (gap > 0) {
      ans += std::to_string(gap);
      gap = 0;
    }
  };
  for (int i = 0; i < kNumRows; ++i) {
    for (int j = 0;j < kNumColumns; ++j){ 
      if (board_[i][j] != Piece::EmptyPiece()) {
        append_gap();
        ans += board_[i][j].ToLetter();
      } else {
        ++gap;
      }
    }
    append_gap();
    if (i < kNumRows - 1) ans += "/";
  }
  return ans;
}
