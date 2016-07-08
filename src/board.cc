#include "board.h"

#include <cassert>
#include <iostream>
#include <map>

#include "bittables.h"

using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

namespace blur {

namespace {

constexpr std::initializer_list<Side> all_sides{Side::kRed, Side::kBlack};
constexpr std::initializer_list<PieceType> all_piece_types{
    PieceType::kRook,     PieceType::kHorse,     PieceType::kCannon,
    PieceType::kElephant, PieceType::kAssistant, PieceType::kKing,
    PieceType::kPawn};

}  // namespace

Board::Board() {
  assert(
      SetBoard("rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR"));
}

bool Board::SetBoard(const string& fen) {
  map<char, Piece> char_piece_map;
  for (auto side : all_sides) {
    for (auto piece_type : all_piece_types) {
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
    for (int j = 0; j < kNumColumns; ++j) {
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

void Board::DebugPrint() const {
  cout << "   a b c d e f g h i" << endl << endl;
  for (int i = kNumRows - 1; i >= 0; --i) {
    cout << i << "  ";
    for (int j = 0; j < kNumColumns; ++j) {
      if (board_[i][j] == Piece::EmptyPiece()) {
        cout << ".";
      } else {
        cout << board_[i][j].ToLetter();
      }
      cout << " ";
    }
    cout << " " << i << endl;
  }
  cout << endl << "   a b c d e f g h i" << endl;
}

namespace {

// Helper function to add all moves to a vector whose start positions are fixed
// and whose end positions are given in a BitBoard.
void InsertMoves(Position from, BitBoard to, vector<Move>* moves) {
  for (auto iter = to.Positions(); iter.HasNext();) {
    moves->emplace_back(from, iter.Next());
  }
}

void GenerateRookMoves(BitBoard rooks, BitBoard all, BitBoard allowed,
                       vector<Move>* moves) {
  for (auto iter = rooks.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    auto to = BitTables::rook_row_moves[from.value()]
                                       [all.GetRowOccupancy(from.Row())];
    to |= BitTables::rook_col_moves[from.value()]
                                   [all.GetColOccupancy(from.Column())];
    to &= allowed;
    InsertMoves(from, to, moves);
  }
}

void GenerateHorseMoves(BitBoard source, BitBoard all, BitBoard allowed,
                        vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    auto to = BitTables::horse_moves[from.value()][all.GetHorseOccupancy(from)];
    to &= allowed;
    InsertMoves(from, to, moves);
  }
}

void GenerateCannonMoves(BitBoard source, BitBoard all, BitBoard allowed,
                         vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    auto to = BitTables::cannon_row_moves[from.value()]
                                         [all.GetRowOccupancy(from.Row())];
    to |= BitTables::cannon_col_moves[from.value()]
                                     [all.GetColOccupancy(from.Column())];
    to &= allowed;
    InsertMoves(from, to, moves);
  }
}

void GenerateElephantMoves(BitBoard source, BitBoard all, BitBoard allowed,
                           vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    auto to =
        BitTables::elephant_moves[from.value()][all.GetElephantOccupancy(from)];
    to &= allowed;
    InsertMoves(from, to, moves);
  }
}

void GenerateSimplePieceMoves(BitBoard source, BitBoard allowed,
                              const std::array<BitBoard, kNumPositions>& table,
                              vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    auto to = table[from.value()];
    to &= allowed;
    InsertMoves(from, to, moves);
  }
}

void GenerateAssistantMoves(BitBoard source, BitBoard allowed,
                            vector<Move>* moves) {
  GenerateSimplePieceMoves(source, allowed, BitTables::assistant_moves, moves);
}

void GenerateRedPawnMoves(BitBoard source, BitBoard allowed,
                          vector<Move>* moves) {
  GenerateSimplePieceMoves(source, allowed, BitTables::red_pawn_moves, moves);
}

void GenerateBlackPawnMoves(BitBoard source, BitBoard allowed,
                            vector<Move>* moves) {
  GenerateSimplePieceMoves(source, allowed, BitTables::black_pawn_moves, moves);
}

void GenerateKingMoves(BitBoard source, BitBoard all, BitBoard allowed,
                       BitBoard other_king, vector<Move>* moves) {
  // Case 1: normal king moves.
  GenerateSimplePieceMoves(source, allowed, BitTables::king_moves, moves);
  // Case 2: special king-to-king move.
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    auto to = BitTables::rook_col_moves[from.value()]
                                       [all.GetColOccupancy(from.Column())];
    to &= other_king;
    InsertMoves(from, to, moves);
  }
}

}  // namespace

vector<Move> Board::GenerateMoves(Side side) const {
  BitBoard all_pieces = BitBoard::EmptyBoard();
  BitBoard allowed_dests = ~BitBoard::EmptyBoard();
  for (auto s : all_sides) {
    for (auto pt : all_piece_types) {
      Piece piece(s, pt);
      all_pieces |= piece_bitboards_[piece.value()];
      if (side == s) allowed_dests &= ~piece_bitboards_[piece.value()];
    }
  }

  vector<Move> moves;
  moves.reserve(50);
  // Rook
  GenerateRookMoves(piece_bitboards_[Piece(side, PieceType::kRook).value()],
                    all_pieces, allowed_dests, &moves);
  // Horse
  GenerateHorseMoves(piece_bitboards_[Piece(side, PieceType::kHorse).value()],
                     all_pieces, allowed_dests, &moves);
  // Cannon
  GenerateCannonMoves(piece_bitboards_[Piece(side, PieceType::kCannon).value()],
                      all_pieces, allowed_dests, &moves);
  // Elephant
  GenerateElephantMoves(
      piece_bitboards_[Piece(side, PieceType::kElephant).value()], all_pieces,
      allowed_dests, &moves);
  // Assistant
  GenerateAssistantMoves(
      piece_bitboards_[Piece(side, PieceType::kAssistant).value()],
      allowed_dests, &moves);
  // Pawn
  if (side == Side::kRed) {
    GenerateRedPawnMoves(
        piece_bitboards_[Piece(side, PieceType::kPawn).value()], allowed_dests,
        &moves);
  } else {
    GenerateBlackPawnMoves(
        piece_bitboards_[Piece(side, PieceType::kPawn).value()], allowed_dests,
        &moves);
  }
  // King
  GenerateKingMoves(
      piece_bitboards_[Piece(side, PieceType::kKing).value()], all_pieces,
      allowed_dests,
      piece_bitboards_[Piece(OtherSide(side), PieceType::kKing).value()],
      &moves);

  return moves;
}

}  // namespace blur
