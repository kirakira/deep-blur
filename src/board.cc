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
  history_.reserve(200);
}

bool Board::SetBoard(const string& fen) {
  static map<char, Piece> char_piece_map = []() {
    map<char, Piece> cpm;
    for (auto side : all_sides) {
      for (auto piece_type : all_piece_types) {
        Piece piece(side, piece_type);
        cpm[piece.ToLetter()] = piece;
      }
    }
    return cpm;
  }();
  static vector<int> max_piece_count = []() {
    vector<int> mpc(8);
    mpc[static_cast<int>(PieceType::kKing)] = 1;
    mpc[static_cast<int>(PieceType::kPawn)] = 5;
    mpc[static_cast<int>(PieceType::kAssistant)] = 2;
    mpc[static_cast<int>(PieceType::kElephant)] = 2;
    mpc[static_cast<int>(PieceType::kHorse)] = 2;
    mpc[static_cast<int>(PieceType::kCannon)] = 2;
    mpc[static_cast<int>(PieceType::kRook)] = 2;
    return mpc;
  }();

  int row = 9, col = 0;
  vector<int> piece_count(32, 0);
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
      } else if (char_piece_map[c] == Piece(Side::kBlack, PieceType::kKing)) {
        if (!(row >= 7 && col >= 3 && col <= 5)) return false;
      }
      if (col >= kNumColumns) return false;
      ++col;
      ++piece_count[char_piece_map[c].value()];
    }
  }
  if (row != 0) return false;
  if (col != kNumColumns) return false;
  for (auto s : all_sides) {
    if (piece_count[Piece(s, PieceType::kKing).value()] != 1) return false;
    for (auto pt : all_piece_types) {
      if (piece_count[Piece(s, pt).value()] >
          max_piece_count[static_cast<int>(pt)]) {
        return false;
      }
    }
  }

  for (int i = 0; i < kNumPositions; ++i) board_[i] = Piece::EmptyPiece();
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
      board_[Position(row, col).value()] = piece;
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
      if (board_[Position(i, j).value()] != Piece::EmptyPiece()) {
        append_gap();
        ans += board_[Position(i, j).value()].ToLetter();
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
      if (board_[Position(i, j).value()] == Piece::EmptyPiece()) {
        cout << ".";
      } else {
        cout << board_[Position(i, j).value()].ToLetter();
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

BitBoard RookDestinations(Position from, BitBoard board,
                          BitBoard allowed_mask) {
  auto to = BitTables::rook_row_moves[from.value()]
                                     [board.GetRowOccupancy(from.Row())];
  to |= BitTables::rook_col_moves[from.value()]
                                 [board.GetColOccupancy(from.Column())];
  to &= allowed_mask;
  return to;
}

void GenerateRookMoves(BitBoard rooks, BitBoard all, BitBoard allowed,
                       vector<Move>* moves) {
  for (auto iter = rooks.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    InsertMoves(from, RookDestinations(from, all, allowed), moves);
  }
}

BitBoard HorseDestinations(Position from, BitBoard board,
                           BitBoard allowed_mask) {
  auto to = BitTables::horse_moves[from.value()][board.GetHorseOccupancy(from)];
  to &= allowed_mask;
  return to;
}

void GenerateHorseMoves(BitBoard source, BitBoard all, BitBoard allowed,
                        vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    InsertMoves(from, HorseDestinations(from, all, allowed), moves);
  }
}

BitBoard HorseReverseDestinations(Position from, BitBoard board,
                                  BitBoard allowed_mask) {
  auto to = BitTables::horse_reverse_moves[from.value()]
                                          [board.GetElephantOccupancy(from)];
  to &= allowed_mask;
  return to;
}

BitBoard CannonDestinations(Position from, BitBoard board,
                            BitBoard allowed_mask) {
  auto to = BitTables::cannon_row_moves[from.value()]
                                       [board.GetRowOccupancy(from.Row())];
  to |= BitTables::cannon_col_moves[from.value()]
                                   [board.GetColOccupancy(from.Column())];
  to &= allowed_mask;
  return to;
}

void GenerateCannonMoves(BitBoard source, BitBoard all, BitBoard allowed,
                         vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    InsertMoves(from, CannonDestinations(from, all, allowed), moves);
  }
}

BitBoard ElephantDestinations(Position from, BitBoard board,
                              BitBoard allowed_mask) {
  auto to =
      BitTables::elephant_moves[from.value()][board.GetElephantOccupancy(from)];
  to &= allowed_mask;
  return to;
}

void GenerateElephantMoves(BitBoard source, BitBoard all, BitBoard allowed,
                           vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    InsertMoves(from, ElephantDestinations(from, all, allowed), moves);
  }
}

BitBoard SimplePieceDestinations(
    Position from, BitBoard allowed_mask,
    const std::array<BitBoard, kNumPositions>& table) {
  auto to = table[from.value()];
  to &= allowed_mask;
  return to;
}

void GenerateSimplePieceMoves(BitBoard source, BitBoard allowed,
                              const std::array<BitBoard, kNumPositions>& table,
                              vector<Move>* moves) {
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    InsertMoves(from, SimplePieceDestinations(from, allowed, table), moves);
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

BitBoard KingSlidingDestinations(Position from, BitBoard board,
                                 BitBoard allowed_mask) {
  auto to = BitTables::rook_col_moves[from.value()]
                                     [board.GetColOccupancy(from.Column())];
  to &= allowed_mask;
  return to;
}

void GenerateKingMoves(BitBoard source, BitBoard all, BitBoard allowed,
                       BitBoard other_king, vector<Move>* moves) {
  // Case 1: normal king moves.
  GenerateSimplePieceMoves(source, allowed, BitTables::king_moves, moves);
  // Case 2: special king-to-king move.
  for (auto iter = source.Positions(); iter.HasNext();) {
    const auto from = iter.Next();
    InsertMoves(from, KingSlidingDestinations(from, all, other_king), moves);
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

std::pair<bool, Position> Board::IsAttacked(Position pos) const {
  DCHECK(board_[pos.value()] != Piece::EmptyPiece());
  const Side my_side = board_[pos.value()].side(),
             other_side = OtherSide(my_side);

  BitBoard all_pieces = BitBoard::EmptyBoard();
  for (auto s : all_sides) {
    for (auto pt : all_piece_types) {
      Piece piece(s, pt);
      all_pieces |= piece_bitboards_[piece.value()];
    }
  }

  const bool in_local_half =
      my_side == Side::kRed ? pos.InRedHalf() : !pos.InRedHalf();

#define RETURN_IF_NONEMPTY(x)                                \
  {                                                          \
    const auto value = x;                                    \
    if (value != BitBoard::EmptyBoard()) {                   \
      return std::make_pair(true, value.Positions().Next()); \
    }                                                        \
  }
  // Pawn
  if (my_side == Side::kRed) {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kPawn).value()],
        BitTables::black_pawn_reverse_moves));
  } else {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kPawn).value()],
        BitTables::red_pawn_reverse_moves));
  }
  // Assistant
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kAssistant).value()],
        BitTables::assistant_moves));
  }
  // Elephant
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(ElephantDestinations(
        pos, all_pieces,
        piece_bitboards_[Piece(other_side, PieceType::kElephant).value()]));
  }
  // Horse
  RETURN_IF_NONEMPTY(HorseReverseDestinations(
      pos, all_pieces,
      piece_bitboards_[Piece(other_side, PieceType::kHorse).value()]));
  // Cannon
  RETURN_IF_NONEMPTY(CannonDestinations(
      pos, all_pieces,
      piece_bitboards_[Piece(other_side, PieceType::kCannon).value()]));
  // Rook
  RETURN_IF_NONEMPTY(RookDestinations(
      pos, all_pieces,
      piece_bitboards_[Piece(other_side, PieceType::kRook).value()]));
  // King
  // Case 1: normal king moves.
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kKing).value()],
        BitTables::king_moves));
  }
  // Case 2: special king-to-king move.
  if (in_local_half && board_[pos.value()].type() == PieceType::kKing) {
    RETURN_IF_NONEMPTY(KingSlidingDestinations(
        pos, all_pieces,
        piece_bitboards_[Piece(other_side, PieceType::kKing).value()]));
  }
#undef RETURN_IF_NONEMPTY
  return std::make_pair(false, Position());
}

void Board::Make(Move move) {
  const Piece from_piece = board_[move.from().value()],
              to_piece = board_[move.to().value()];
  DCHECK(from_piece != Piece::EmptyPiece());
  DCHECK(from_piece != to_piece);
  // 1. Update history.
  history_.push_back({move, to_piece});
  // 2. Update bitboards.
  piece_bitboards_[from_piece.value()].Make(move);
  if (to_piece != Piece::EmptyPiece()) {
    piece_bitboards_[to_piece.value()] &= ~BitBoard::Fill(move.to());
  }
  // 3. Update board_.
  board_[move.to().value()] = board_[move.from().value()];
  board_[move.from().value()] = Piece::EmptyPiece();
}

void Board::Unmake() {
  DCHECK(!history_.empty());
  const auto history_move = history_.back();
  const auto move = history_move.move;
  const auto from_piece = board_[move.to().value()],
             to_piece = history_move.capture;
  // 1. Restore board_.
  board_[move.from().value()] = from_piece;
  board_[move.to().value()] = to_piece;
  // 2. Restore bitboards.
  if (to_piece != Piece::EmptyPiece()) {
    piece_bitboards_[to_piece.value()] |= BitBoard::Fill(move.to());
  }
  piece_bitboards_[from_piece.value()].Unmake(move);
  // 3. Restore history_;
  history_.pop_back();
}

bool Board::InCheck(Side side) const {
  return IsAttacked(piece_bitboards_[Piece(side, PieceType::kKing).value()]
                        .Positions()
                        .Next())
      .first;
}

}  // namespace blur
