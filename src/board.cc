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

Board::Board() : eval_(Evaluator::Make()) {
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
  repetition_start_[0] = repetition_start_[1] = 0;

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
      MutablePieceAt(pos) = piece;
      piece_bitboards_[piece.value()] |= BitBoard::Fill(Position(row, col));
      hash_ ^= BoardHash::piece_position_hash[piece.value()][pos.value()];

      ++col;
    }
  }

  eval_->SetBoard(board_);

  return true;
}

Piece Board::PieceAt(Position pos) const { return board_[pos.value()]; }

Piece& Board::MutablePieceAt(Position pos) { return board_[pos.value()]; }

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
      if (PieceAt(Position(i, j)) != Piece::EmptyPiece()) {
        append_gap();
        ans += PieceAt(Position(i, j)).ToLetter();
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
      if (PieceAt(Position(i, j)) == Piece::EmptyPiece()) {
        cout << ".";
      } else {
        cout << PieceAt(Position(i, j)).ToLetter();
      }
      cout << " ";
    }
    cout << " " << i << endl;
  }
  cout << endl << "   a b c d e f g h i" << endl << endl;
  cout << "hash: " << hash_ << ", "
       << "eval: " << eval_->CurrentScore() << endl;
}

namespace {

BitBoard RookDestinations(BitBoard board, Position from) {
  auto to =
      kBitTables
          .rook_row_moves[from.value()][board.GetRowOccupancy(from.Row())];
  to |= kBitTables
            .rook_col_moves[from.value()][board.GetColOccupancy(from.Column())];
  return to;
}

BitBoard HorseDestinations(BitBoard board, Position from) {
  return kBitTables.horse_moves[from.value()][board.GetHorseOccupancy(from)];
}

BitBoard HorseReverseDestinations(BitBoard board, Position from) {
  return kBitTables
      .horse_reverse_moves[from.value()][board.GetElephantOccupancy(from)];
}

BitBoard CannonDestinations(BitBoard board, Position from) {
  auto to =
      kBitTables
          .cannon_row_moves[from.value()][board.GetRowOccupancy(from.Row())];
  to |=
      kBitTables
          .cannon_col_moves[from.value()][board.GetColOccupancy(from.Column())];
  return to;
}

BitBoard ElephantDestinations(BitBoard board, Position from) {
  return kBitTables
      .elephant_moves[from.value()][board.GetElephantOccupancy(from)];
}

BitBoard AssistantDestinations(Position from) {
  return kBitTables.assistant_moves[from.value()];
}

BitBoard RedPawnDestinations(Position from) {
  return kBitTables.red_pawn_moves[from.value()];
}

BitBoard BlackPawnDestinations(Position from) {
  return kBitTables.black_pawn_moves[from.value()];
}

BitBoard RedPawnReverseDestinations(Position from) {
  return kBitTables.red_pawn_reverse_moves[from.value()];
}

BitBoard BlackPawnReverseDestinations(Position from) {
  return kBitTables.black_pawn_reverse_moves[from.value()];
}

BitBoard KingNormalDestinations(Position from) {
  return kBitTables.king_moves[from.value()];
}

BitBoard KingSlidingDestinations(BitBoard board, BitBoard other_king,
                                 Position from) {
  auto to =
      kBitTables
          .rook_col_moves[from.value()][board.GetColOccupancy(from.Column())];
  to &= other_king;
  return to;
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

template <typename DestinationFunction, typename VisitFunction>
void VisitDestinations(BitBoard source, DestinationFunction dest,
                       VisitFunction visitor) {
  source.VisitPositions(
      [dest, visitor](Position from) { visitor(from, dest(from)); });
}

}  // namespace

BitBoard Board::AllPiecesMask() const {
  return ComputePiecesMask(piece_bitboards_, [](Piece) { return true; });
}

BitBoard Board::SidePiecesMask(Side side) const {
  return ComputePiecesMask(piece_bitboards_,
                           [side](Piece p) { return p.side() == side; });
}

template <typename Function>
void Board::VisitMoves(Side side, Function visitor) const {
  const BitBoard all_pieces = AllPiecesMask();
  // Rook
  VisitDestinations(piece_bitboards_[Piece(side, PieceType::kRook).value()],
                    CurryFront(RookDestinations, all_pieces), visitor);
  // Horse
  VisitDestinations(piece_bitboards_[Piece(side, PieceType::kHorse).value()],
                    CurryFront(HorseDestinations, all_pieces), visitor);
  // Cannon
  VisitDestinations(piece_bitboards_[Piece(side, PieceType::kCannon).value()],
                    CurryFront(CannonDestinations, all_pieces), visitor);
  // Elephant
  VisitDestinations(piece_bitboards_[Piece(side, PieceType::kElephant).value()],
                    CurryFront(ElephantDestinations, all_pieces), visitor);
  // Assistant
  VisitDestinations(
      piece_bitboards_[Piece(side, PieceType::kAssistant).value()],
      AssistantDestinations, visitor);
  // Pawn
  if (side == Side::kRed) {
    VisitDestinations(piece_bitboards_[Piece(side, PieceType::kPawn).value()],
                      RedPawnDestinations, visitor);
  } else {
    VisitDestinations(piece_bitboards_[Piece(side, PieceType::kPawn).value()],
                      BlackPawnDestinations, visitor);
  }
  // King
  VisitDestinations(piece_bitboards_[Piece(side, PieceType::kKing).value()],
                    KingNormalDestinations, visitor);
  VisitDestinations(
      piece_bitboards_[Piece(side, PieceType::kKing).value()],
      CurryFront(
          KingSlidingDestinations, all_pieces,
          piece_bitboards_[Piece(OtherSide(side), PieceType::kKing).value()]),
      visitor);
}

MoveList Board::GenerateMovesWithAllowedMask(Side side,
                                             BitBoard allowed) const {
  MoveList moves;
  const auto verify_and_insert_move = [allowed, &moves](Position from,
                                                        BitBoard dests) {
    dests &= allowed;
    dests.VisitPositions([from, &moves](Position to) { moves.Add(from, to); });
  };
  VisitMoves(side, verify_and_insert_move);
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
  DCHECK(PieceAt(pos) != Piece::EmptyPiece());
  const Side my_side = PieceAt(pos).side(), other_side = OtherSide(my_side);

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
    RETURN_IF_NONEMPTY(
        BlackPawnReverseDestinations(pos) &
        piece_bitboards_[Piece(other_side, PieceType::kPawn).value()]);
  } else {
    RETURN_IF_NONEMPTY(
        RedPawnReverseDestinations(pos) &
        piece_bitboards_[Piece(other_side, PieceType::kPawn).value()]);
  }
  // Assistant
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(
        AssistantDestinations(pos) &
        piece_bitboards_[Piece(other_side, PieceType::kAssistant).value()]);
  }
  // Elephant
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(
        ElephantDestinations(all_pieces, pos) &
        piece_bitboards_[Piece(other_side, PieceType::kElephant).value()]);
  }
  // Horse
  RETURN_IF_NONEMPTY(
      HorseReverseDestinations(all_pieces, pos) &
      piece_bitboards_[Piece(other_side, PieceType::kHorse).value()]);
  // Cannon
  RETURN_IF_NONEMPTY(
      CannonDestinations(all_pieces, pos) &
      piece_bitboards_[Piece(other_side, PieceType::kCannon).value()]);
  // Rook
  RETURN_IF_NONEMPTY(
      RookDestinations(all_pieces, pos) &
      piece_bitboards_[Piece(other_side, PieceType::kRook).value()]);
  // King
  // Case 1: normal king moves.
  if (!in_local_half) {
    RETURN_IF_NONEMPTY(
        KingNormalDestinations(pos) &
        piece_bitboards_[Piece(other_side, PieceType::kKing).value()]);
  }
  // Case 2: special king-to-king move.
  if (in_local_half && PieceAt(pos).type() == PieceType::kKing) {
    RETURN_IF_NONEMPTY(KingSlidingDestinations(
        all_pieces,
        piece_bitboards_[Piece(other_side, PieceType::kKing).value()], pos));
  }
#undef RETURN_IF_NONEMPTY
  return std::make_pair(false, Position());
}

MoveType Board::MakeWithoutRepetitionDetection(Move move) {
  const Piece from_piece = PieceAt(move.from()), to_piece = PieceAt(move.to());
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
  MutablePieceAt(move.to()) = PieceAt(move.from());
  MutablePieceAt(move.from()) = Piece::EmptyPiece();
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
  eval_->OnMake(move, from_piece, to_piece);

  if (to_piece == Piece::EmptyPiece()) {
    return MoveType::kRegular;
  } else if (to_piece.type() == PieceType::kKing) {
    return MoveType::kKingCapture;
  } else {
    return MoveType::kCapture;
  }
}

MoveType Board::Make(Move move) {
  MoveType move_type = MakeWithoutRepetitionDetection(move);
  // Determine repetition type if this is not a capture move.
  if (move_type == MoveType::kRegular) {
    const Side moving_side = PieceAt(move.from()).side();
    for (int i = static_cast<int>(history_.size()) - 1;
         i >= std::max(repetition_start_[static_cast<int>(moving_side)],
                       irreversible_moves_.back() + 1);
         --i) {
      if (history_[i].hash_before_move == hash_) {
        move_type = GetRepetitionType(i);
#ifndef NDEBUG
        if (move_type != MoveType::kRepetition) {
          std::cerr << "Repetition detected: " << static_cast<int>(move_type)
                    << endl;
          DebugPrint();
          for (int j = i; j < static_cast<int>(history_.size()); ++j) {
            std::cerr << history_[j].move.ToString() << " ";
          }
          std::cerr << endl;
        }
#endif
        break;
      }
    }
  }
  return move_type;
}

BitBoard Board::ControlledPositions(Side side) const {
  BitBoard controlled = BitBoard::EmptyBoard();
  const auto append_to_controlled = [&controlled](Position, BitBoard dests) {
    controlled |= dests;
  };
  VisitMoves(side, append_to_controlled);
  return controlled;
}

bool Board::IsChasing(Position pos) const {
  const Piece source_piece = PieceAt(pos);
  const Side my_side = source_piece.side(), other_side = OtherSide(my_side);
  const BitBoard all_pieces = AllPiecesMask();
  if (source_piece.type() == PieceType::kCannon) {
    if (IsAttacked(pos).first) return false;
    const auto attacked_positions = CannonDestinations(all_pieces, pos);
    // Cannon to attack a rook.
    if (BitBoard::EmptyBoard() !=
        (attacked_positions &
         piece_bitboards_[Piece(other_side, PieceType::kRook).value()])) {
      return true;
    }
    // Cannon to attack an unprotected horse.
    const auto other_horses =
        piece_bitboards_[Piece(other_side, PieceType::kHorse).value()];
    if (BitBoard::EmptyBoard() != (other_horses & attacked_positions &
                                   ~ControlledPositions(other_side))) {
      return true;
    }
  } else if (source_piece.type() == PieceType::kHorse) {
    if (IsAttacked(pos).first) return false;
    const auto attacked_positions = HorseDestinations(all_pieces, pos);
    // Horse to attack a rook.
    if (BitBoard::EmptyBoard() !=
        (HorseDestinations(all_pieces, pos) &
         piece_bitboards_[Piece(other_side, PieceType::kRook).value()])) {
      return true;
    }
    // Horse to attack an unprotected cannon.
    const auto other_cannons =
        piece_bitboards_[Piece(other_side, PieceType::kCannon).value()];
    if (BitBoard::EmptyBoard() != (other_cannons & attacked_positions &
                                   ~ControlledPositions(other_side))) {
      return true;
    }
  } else if (source_piece.type() == PieceType::kRook) {
    // Rook to chase an unprotected cannon or horse.
    const auto attacked_positions = RookDestinations(all_pieces, pos);
    BitBoard target_pieces =
        piece_bitboards_[Piece(other_side, PieceType::kCannon).value()] |
        piece_bitboards_[Piece(other_side, PieceType::kHorse).value()];
    const BitBoard controlled_positions = ControlledPositions(other_side);
    if (controlled_positions[pos]) return false;
    target_pieces &= ~controlled_positions;
    return BitBoard::EmptyBoard() != (attacked_positions & target_pieces);
  }
  return false;
}

MoveType Board::GetRepetitionType(int first_move_index) {
  DCHECK(!history_.empty() &&
         first_move_index <= static_cast<int>(history_.size()) - 2);

  std::vector<Move> unwinded_moves;
  unwinded_moves.reserve(
      std::max(0, static_cast<int>(history_.size()) - first_move_index));

  const Side last_move_side = PieceAt(history_.back().move.to()).side();
  Side last_side = OtherSide(last_move_side);
  bool perpetual_check[2] = {true, true}, perpetual_attack[2] = {true, true};
  bool early_break = false;
  while (static_cast<int>(history_.size()) > first_move_index) {
    const Move current_move = history_.back().move;
    const Side move_side = PieceAt(current_move.to()).side();
    // If one side has made two moves in a row, abort.
    if (move_side == last_side) {
      early_break = true;
      break;
    }
    last_side = move_side;

    // Test for check and chase.
    int move_side_int = static_cast<int>(move_side);
    const bool is_checking = InCheck(OtherSide(move_side));
    const bool is_chasing = IsChasing(current_move.to());
    perpetual_check[move_side_int] =
        perpetual_check[move_side_int] && is_checking;
    perpetual_attack[move_side_int] =
        perpetual_attack[move_side_int] && (is_checking || is_chasing);

    // Test for early break.
    if (perpetual_check[0] == false && perpetual_check[1] == false &&
        perpetual_attack[0] == false && perpetual_attack[1] == false) {
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
  } else if (perpetual_attack[0] == perpetual_attack[1]) {
    return MoveType::kRepetition;
  } else if (perpetual_attack[static_cast<int>(last_move_side)] &&
             !perpetual_attack[static_cast<int>(OtherSide(last_move_side))]) {
    return MoveType::kPerpetualAttacker;
  } else {
    return MoveType::kPerpetualAttackee;
  }
}

void Board::Unmake() {
  DCHECK(!history_.empty());
  const auto history_move = history_.back();
  const auto move = history_move.move;
  const auto from_piece = PieceAt(move.to()), to_piece = history_move.capture;
  // 1. Update board evaluation.
  eval_->OnUnmake(move, from_piece, to_piece);
  // 2. Restore hash_.
  hash_ = history_move.hash_before_move;
  // 3. Restore board_.
  MutablePieceAt(move.from()) = from_piece;
  MutablePieceAt(move.to()) = to_piece;
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

void Board::ResetRepetitionHistory(Side side) {
  repetition_start_[static_cast<int>(side)] = static_cast<int>(history_.size());
}

bool Board::IsPseudoLegalMove(Side side, Move move) const {
  const auto from = move.from(), to = move.to();
  const Piece from_piece = PieceAt(from);
  if (from_piece == Piece::EmptyPiece() || from_piece.side() != side) {
    return false;
  }

  const BitBoard all_pieces = AllPiecesMask();
  BitBoard dests = BitBoard::EmptyBoard();

  switch (from_piece.type()) {
    case PieceType::kKing: {
      dests = KingNormalDestinations(from);
      dests |= KingSlidingDestinations(
          all_pieces,
          piece_bitboards_[Piece(OtherSide(side), PieceType::kKing).value()],
          from);
    } break;
    case PieceType::kPawn: {
      if (side == Side::kRed) {
        dests = RedPawnDestinations(from);
      } else {
        dests = BlackPawnDestinations(from);
      }
    } break;
    case PieceType::kAssistant: {
      dests = AssistantDestinations(from);
    } break;
    case PieceType::kElephant: {
      dests = ElephantDestinations(all_pieces, from);
    } break;
    case PieceType::kHorse: {
      dests = HorseDestinations(all_pieces, from);
    } break;
    case PieceType::kCannon: {
      dests = CannonDestinations(all_pieces, from);
    } break;
    case PieceType::kRook: {
      dests = RookDestinations(all_pieces, from);
    } break;
  }

  const BitBoard allowed_dests =
      ~BitBoard::EmptyBoard() & ~SidePiecesMask(side);
  dests &= allowed_dests;
  if (!dests[to]) return false;

  return true;
}

std::pair<bool, MoveType> Board::CheckedMake(Side side, Move move) {
  if (!IsPseudoLegalMove(side, move)) {
    return std::make_pair(false, MoveType::kRegular);
  }

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
