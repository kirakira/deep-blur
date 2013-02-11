package xiangqi.ai;

import java.util.Map;
import java.util.HashMap;
import java.util.List;

public class Agent {
    protected int turn;
    public Board board;

    // Value[0]: depth
    // Value[1]: score (relative)
    // Value[2]: best move
    protected Map<Long, int[]> transposition = new HashMap<Long, int[]>();

    public Agent() {
        board = new Board();
        turn = 0;
    }

    public Agent(int[][] board, int turn) {
        this.board = new Board(board);
        this.turn = turn;
    }

    protected void turn() {
        turn = 1 - turn;
    }

    public void move(Move move) {
        board.move(move.internalRepresentation());
        turn();
    }

    public void unmove() {
        board.unmove();
        turn();
    }

    public Move search() {
        System.out.println("Score: " + search(5, turn));
        return new Move(transposition.get(board.currentHash(turn))[2]);
    }

    protected int search(int depth, int turn) {
        if (depth == 0)
            return evaluate(turn);

        int[] history = transposition.get(board.currentHash(turn));
        if (history != null && history[0] >= depth)
            return history[1];

        List<Integer> moves = board.generateMoves(turn);
        int best = Integer.MAX_VALUE, bestMove = 0;
        for (int move: moves) {
            board.move(move);
            int t = search(depth - 1, 1 - turn);
            board.unmove();
            if (t < best) {
                best = t;
                bestMove = move;
            }
        }

        int score = -best;

        transposition.put(board.currentHash(turn), new int[] {depth, score, bestMove});
        return score;
    }

    protected int evaluate(int turn) {
        return board.staticValue(turn);
    }
}
