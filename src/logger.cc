#include "logger.h"

namespace blur {

void Logger::Print() const {
  using std::cout;
  using std::endl;
  for (const auto& node : nodes_) {
    cout << "node " << node.id << " parent " << node.parent_id << " from_move "
         << node.from_move.ToString() << " "
         << (node.side == Side::kRed ? "r" : "b") << " depth " << node.depth
         << " alpha " << node.alpha << " beta " << node.beta << " final tt? "
         << node.tt_hit << " score " << node.score << " "
         << node.best_move.ToString() << endl;
  }
}

void Logger::SaveNode(const SearchNodeInfo& info) { nodes_.push_back(info); }

}  // namespace blur
