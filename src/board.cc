#include "board.h"

#include <iostream>
#include <map>

#include "bittables.h"
#include "board-hash.h"

using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

namespace blur {

namespace {

constexpr BitTables kBitTables{};
// Use static_assert to make sure the tables are generated in compile-time.
static_assert(kBitTables.rook_col_moves[89][1023] ==
                  BitBoard::Fill(Position(80)),
              "Bad bittables");

constexpr std::initializer_list<Side> all_sides{Side::kRed, Side::kBlack};
constexpr std::initializer_list<PieceType> all_piece_types{
    PieceType::kRook,     PieceType::kHorse,     PieceType::kCannon,
    PieceType::kElephant, PieceType::kAssistant, PieceType::kKing,
    PieceType::kPawn};

}  // namespace

Board::Board() {
  DCHECK(
      SetBoard("rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR"));
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
  history_.clear();
  history_.reserve(200);
  irreversible_moves_.clear();
  history_.reserve(50);
  irreversible_moves_.push_back(-1);
  hash_ = 0;
  repetition_start_ = 0;

  row = 9, col = 0;
  for (char c : fen) {
    if (c >= '0' && c <= '9') {
      col += (c - '0');
    } else if (c == '/') {
      --row;
      col = 0;
    } else {
      Position pos(row, col);
      Piece piece = char_piece_map[c];
      board_[pos.value()] = piece;
      piece_bitboards_[piece.value()] |= BitBoard::Fill(Position(row, col));
      hash_ ^= BoardHash::piece_position_hash[piece.value()][pos.value()];

      ++col;
    }
  }

  eval_.SetBoard(board_);

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
  cout << "hash: " << hash_ << endl;
  cout << "eval: " << eval_.CurrentScore() << endl;
}

namespace {

// Helper function to add all moves to a vector whose start positions are fixed
// and whose end positions are given in a BitBoard.
void InsertMoves(Position from, BitBoard to, MoveList* moves) {
  to.VisitPositions([from, moves](Position pos) { moves->Add(from, pos); });
}

BitBoard RookDestinations(Position from, BitBoard board,
                          BitBoard allowed_mask) {
  auto to =
      kBitTables
          .rook_row_moves[from.value()][board.GetRowOccupancy(from.Row())];
  to |= kBitTables
            .rook_col_moves[from.value()][board.GetColOccupancy(from.Column())];
  to &= allowed_mask;
  return to;
}

void GenerateRookMoves(BitBoard source, BitBoard all, BitBoard allowed,
                       MoveList* moves) {
  source.VisitPositions([all, allowed, moves](Position from) {
    InsertMoves(from, RookDestinations(from, all, allowed), moves);
  });
}

BitBoard HorseDestinations(Position from, BitBoard board,
                           BitBoard allowed_mask) {
  auto to = kBitTables.horse_moves[from.value()][board.GetHorseOccupancy(from)];
  to &= allowed_mask;
  return to;
}

void GenerateHorseMoves(BitBoard source, BitBoard all, BitBoard allowed,
                        MoveList* moves) {
  source.VisitPositions([all, allowed, moves](Position from) {
    InsertMoves(from, HorseDestinations(from, all, allowed), moves);
  });
}

BitBoard HorseReverseDestinations(Position from, BitBoard board,
                                  BitBoard allowed_mask) {
  auto to =
      kBitTables
          .horse_reverse_moves[from.value()][board.GetElephantOccupancy(from)];
  to &= allowed_mask;
  return to;
}

BitBoard CannonDestinations(Position from, BitBoard board,
                            BitBoard allowed_mask) {
  auto to =
      kBitTables
          .cannon_row_moves[from.value()][board.GetRowOccupancy(from.Row())];
  to |=
      kBitTables
          .cannon_col_moves[from.value()][board.GetColOccupancy(from.Column())];
  to &= allowed_mask;
  return to;
}

void GenerateCannonMoves(BitBoard source, BitBoard all, BitBoard allowed,
                         MoveList* moves) {
  source.VisitPositions([all, allowed, moves](Position from) {
    InsertMoves(from, CannonDestinations(from, all, allowed), moves);
  });
}

BitBoard ElephantDestinations(Position from, BitBoard board,
                              BitBoard allowed_mask) {
  auto to =
      kBitTables.elephant_moves[from.value()][board.GetElephantOccupancy(from)];
  to &= allowed_mask;
  return to;
}

void GenerateElephantMoves(BitBoard source, BitBoard all, BitBoard allowed,
                           MoveList* moves) {
  source.VisitPositions([all, allowed, moves](Position from) {
    InsertMoves(from, ElephantDestinations(from, all, allowed), moves);
  });
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
                              MoveList* moves) {
  source.VisitPositions([allowed, &table, moves](Position from) {
    InsertMoves(from, SimplePieceDestinations(from, allowed, table), moves);
  });
}

void GenerateAssistantMoves(BitBoard source, BitBoard allowed,
                            MoveList* moves) {
  GenerateSimplePieceMoves(source, allowed, kBitTables.assistant_moves, moves);
}

void GenerateRedPawnMoves(BitBoard source, BitBoard allowed, MoveList* moves) {
  GenerateSimplePieceMoves(source, allowed, kBitTables.red_pawn_moves, moves);
}

void GenerateBlackPawnMoves(BitBoard source, BitBoard allowed,
                            MoveList* moves) {
  GenerateSimplePieceMoves(source, allowed, kBitTables.black_pawn_moves, moves);
}

BitBoard KingSlidingDestinations(Position from, BitBoard board,
                                 BitBoard allowed_mask) {
  auto to =
      kBitTables
          .rook_col_moves[from.value()][board.GetColOccupancy(from.Column())];
  to &= allowed_mask;
  return to;
}

void GenerateKingMoves(BitBoard source, BitBoard all, BitBoard allowed,
                       BitBoard other_king, MoveList* moves) {
  // Case 1: normal king moves.
  GenerateSimplePieceMoves(source, allowed, kBitTables.king_moves, moves);
  // Case 2: special king-to-king move.
  source.VisitPositions([all, other_king, moves](Position from) {
    InsertMoves(from, KingSlidingDestinations(from, all, other_king), moves);
  });
}

// Return the binary or of all piece bitboards on which f returns true.
template <typename Predicate>
BitBoard ComputePiecesMask(const BitBoard piece_bitboards[], Predicate f) {
  BitBoard ans = BitBoard::EmptyBoard();
  for (auto s : all_sides) {
    for (auto pt : all_piece_types) {
      Piece piece(s, pt);
      if (f(piece)) ans |= piece_bitboards[piece.value()];
    }
  }
  return ans;
}

}  // namespace

BitBoard Board::AllPiecesMask() const {
  return ComputePiecesMask(piece_bitboards_, [](Piece) { return true; });
}

BitBoard Board::SidePiecesMask(Side side) const {
  return ComputePiecesMask(piece_bitboards_,
                           [side](Piece p) { return p.side() == side; });
}

MoveList Board::GenerateMovesWithAllowedMask(Side side,
                                             BitBoard allowed_dests) const {
  const BitBoard all_pieces = AllPiecesMask();

  MoveList moves;
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

MoveList Board::GenerateMoves(Side side) const {
  const BitBoard allowed_dests =
      ~BitBoard::EmptyBoard() & ~SidePiecesMask(side);
  return GenerateMovesWithAllowedMask(side, allowed_dests);
}

MoveList Board::GenerateCaptures(Side side) const {
  const BitBoard allowed_dests = SidePiecesMask(OtherSide(side));
  return GenerateMovesWithAllowedMask(side, allowed_dests);
}

std::pair<bool, Position> Board::IsAttacked(Position pos) const {
  DCHECK(board_[pos.value()] != Piece::EmptyPiece());
  const Side my_side = board_[pos.value()].side(),
             other_side = OtherSide(my_side);

  const BitBoard all_pieces = AllPiecesMask();
  const bool in_local_half =
      my_side == Side::kRed ? pos.InRedHalf() : !pos.InRedHalf();

#define RETURN_IF_NONEMPTY(x)                           \
  {                                                     \
    const auto value = x;                               \
    if (value != BitBoard::EmptyBoard()) {              \
      return std::make_pair(true, value.AnyPosition()); \
    }                                                   \
  }
  // Pawn
  if (my_side == Side::kRed) {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kPawn).value()],
        kBitTables.black_pawn_reverse_moves));
  } else {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kPawn).value()],
        kBitTables.red_pawn_reverse_moves));
  }
  // Assistant
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(SimplePieceDestinations(
        pos, piece_bitboards_[Piece(other_side, PieceType::kAssistant).value()],
        kBitTables.assistant_moves));
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
        kBitTables.king_moves));
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

void Board::MakeWithoutRepetitionDetection(Move move) {
  const Piece from_piece = board_[move.from().value()],
              to_piece = board_[move.to().value()];
  DCHECK(from_piece != Piece::EmptyPiece());
  DCHECK(move.from() != move.to());
  // 1. Update irreversible_moves_.
  if (to_piece != Piece::EmptyPiece()) {
    irreversible_moves_.push_back(static_cast<int>(history_.size()));
  }
  // 2. Update history.
  history_.push_back({hash_, move, to_piece});
  // 3. Update bitboards.
  piece_bitboards_[from_piece.value()].Make(move);
  if (to_piece != Piece::EmptyPiece()) {
    piece_bitboards_[to_piece.value()] &= ~BitBoard::Fill(move.to());
  }
  // 4. Update board_.
  board_[move.to().value()] = board_[move.from().value()];
  board_[move.from().value()] = Piece::EmptyPiece();
  // 5. Update hash_.
  hash_ ^=
      BoardHash::piece_position_hash[from_piece.value()][move.from().value()];
  hash_ ^=
      BoardHash::piece_position_hash[from_piece.value()][move.to().value()];
  if (to_piece != Piece::EmptyPiece()) {
    hash_ ^=
        BoardHash::piece_position_hash[to_piece.value()][move.to().value()];
  }
  // 6. Update board evaluation.
  eval_.OnMake(move, from_piece, to_piece);
}

MoveType Board::Make(Move move) {
  MakeWithoutRepetitionDetection(move);
  // Determine move type.
  MoveType move_type = MoveType::kRegular;
  for (int i = static_cast<int>(history_.size()) - 1;
       i >= std::max(repetition_start_, irreversible_moves_.back() + 1); --i) {
    if (history_[i].hash_before_move == hash_) {
      move_type = GetRepetitionType(i);
      break;
    }
  }
  return move_type;
}

bool Board::IsChasing(Position) const { return false; }

MoveType Board::GetRepetitionType(int first_move_index) {
  DCHECK(!history_.empty() &&
         first_move_index <= static_cast<int>(history_.size()) - 2);

  std::vector<Move> unwinded_moves;
  unwinded_moves.reserve(
      std::max(0, static_cast<int>(history_.size()) - first_move_index));

  const Side last_move_side = board_[history_.back().move.to().value()].side();
  Side last_side = OtherSide(last_move_side);
  bool perpetual_check[2] = {true, true}, perpetual_chase[2] = {true, true};
  bool early_break = false;
  while (static_cast<int>(history_.size()) > first_move_index) {
    const Move current_move = history_.back().move;
    const Side move_side = board_[current_move.to().value()].side();
    // If one side has made two moves in a row, abort.
    if (move_side == last_side) {
      early_break = true;
      break;
    }
    last_side = move_side;

    // Test for check and chase.
    int move_side_int = static_cast<int>(move_side);
    perpetual_check[move_side_int] =
        perpetual_check[move_side_int] && InCheck(OtherSide(move_side));
    perpetual_chase[move_side_int] =
        perpetual_chase[move_side_int] && IsChasing(current_move.to());

    // Test for early break.
    if (perpetual_check[0] == false && perpetual_check[1] == false &&
        perpetual_chase[0] == false && perpetual_chase[1] == false) {
      early_break = true;
      break;
    }

    // Unwind current move.
    unwinded_moves.push_back(current_move);
    Unmake();
  }
  for (auto it = unwinded_moves.rbegin(); it != unwinded_moves.rend(); ++it) {
    MakeWithoutRepetitionDetection(*it);
  }

  if (early_break) {
    return MoveType::kRepetition;
  } else if (perpetual_check[0] && perpetual_check[1]) {
    return MoveType::kRepetition;
  } else if (perpetual_check[static_cast<int>(last_move_side)] &&
             !perpetual_check[static_cast<int>(OtherSide(last_move_side))]) {
    return MoveType::kPerpetualAttacker;
  } else if (perpetual_check[static_cast<int>(OtherSide(last_move_side))] &&
             !perpetual_check[static_cast<int>(last_move_side)]) {
    return MoveType::kPerpetualAttackee;
  } else if (perpetual_chase[0] == perpetual_chase[1]) {
    return MoveType::kRepetition;
  } else if (perpetual_chase[static_cast<int>(last_move_side)] &&
             !perpetual_chase[static_cast<int>(OtherSide(last_move_side))]) {
    return MoveType::kPerpetualAttacker;
  } else {
    return MoveType::kPerpetualAttackee;
  }
}

void Board::Unmake() {
  DCHECK(!history_.empty());
  const auto history_move = history_.back();
  const auto move = history_move.move;
  const auto from_piece = board_[move.to().value()],
             to_piece = history_move.capture;
  // 1. Update board evaluation.
  eval_.OnUnmake(move, from_piece, to_piece);
  // 2. Restore hash_.
  hash_ = history_move.hash_before_move;
  // 3. Restore board_.
  board_[move.from().value()] = from_piece;
  board_[move.to().value()] = to_piece;
  // 4. Restore bitboards.
  if (to_piece != Piece::EmptyPiece()) {
    piece_bitboards_[to_piece.value()] |= BitBoard::Fill(move.to());
  }
  piece_bitboards_[from_piece.value()].Unmake(move);
  // 5. Restore history_.
  history_.pop_back();
  // 6. Restore irreversible_moves_.
  if (irreversible_moves_.back() == static_cast<int>(history_.size())) {
    irreversible_moves_.pop_back();
  }
}

bool Board::InCheck(Side side) const {
  return IsAttacked(piece_bitboards_[Piece(side, PieceType::kKing).value()]
                        .AnyPosition())
      .first;
}

void Board::ResetRepetitionHistory() {
  repetition_start_ = static_cast<int>(history_.size());
}

std::pair<bool, MoveType> Board::CheckedMake(Side side, Move move) {
  const auto from = move.from(), to = move.to();
  const Piece from_piece = board_[from.value()];
  if (from_piece == Piece::EmptyPiece() || from_piece.side() != side) {
    return std::make_pair(false, MoveType::kRegular);
  }

  const BitBoard all_pieces = AllPiecesMask();
  const BitBoard allowed_dests =
      ~BitBoard::EmptyBoard() & ~SidePiecesMask(side);
  BitBoard dests = BitBoard::EmptyBoard();

  switch (from_piece.type()) {
    case PieceType::kKing: {
      dests =
          SimplePieceDestinations(from, allowed_dests, kBitTables.king_moves);
      dests |= KingSlidingDestinations(
          from, all_pieces,
          piece_bitboards_[Piece(OtherSide(side), PieceType::kKing).value()]);
    } break;
    case PieceType::kPawn: {
      if (side == Side::kRed) {
        dests = SimplePieceDestinations(from, allowed_dests,
                                        kBitTables.red_pawn_moves);
      } else {
        dests = SimplePieceDestinations(from, allowed_dests,
                                        kBitTables.black_pawn_moves);
      }
    } break;
    case PieceType::kAssistant: {
      dests = SimplePieceDestinations(from, allowed_dests,
                                      kBitTables.assistant_moves);
    } break;
    case PieceType::kElephant: {
      dests = ElephantDestinations(from, all_pieces, allowed_dests);
    } break;
    case PieceType::kHorse: {
      dests = HorseDestinations(from, all_pieces, allowed_dests);
    } break;
    case PieceType::kCannon: {
      dests = CannonDestinations(from, all_pieces, allowed_dests);
    } break;
    case PieceType::kRook: {
      dests = RookDestinations(from, all_pieces, allowed_dests);
    } break;
  }

  if (!dests[to]) return std::make_pair(false, MoveType::kRegular);

  const MoveType mt = Make(move);
  if (InCheck(side)) {
    Unmake();
    return std::make_pair(false, mt);
  }

  return std::make_pair(true, mt);
}

bool Board::CheckedUnmake() {
  if (history_.empty()) return false;
  Unmake();
  return true;
}

uint64 Board::HashCode(Side side) const {
  return side == Side::kRed ? hash_ : hash_ ^ BoardHash::side_hash;
}

}  // namespace blur
