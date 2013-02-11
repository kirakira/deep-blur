package xiangqi.ai;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.Comparator;
import java.util.Collections;

public class Agent {
    protected int turn;
    protected static final int INFINITY = 100000000;
    public Board board;

    // Value[0]: depth
    // Value[1]: score (relative)
    // Value[2]: best move
    protected Map<Long, int[]> transposition = new HashMap<Long, int[]>();
    protected Map<Integer, Integer> moveScore;

    protected Comparator<Integer> compare = new Comparator<Integer>() {
        public int compare(Integer o1, Integer o2) {
            Integer s1 = moveScore.get(o1);
            int i1, i2;
            if (s1 == null)
                i1 = 0;
            else
                i1 = s1.intValue();

            Integer s2 = moveScore.get(o2);
            if (s2 == null)
                i2 = 0;
            else
                i2 = s2.intValue();

            return i2 - i1;
        }
    };

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

    protected int evaluateCount = 0;

    public Move search() {
        evaluateCount = 0;
        moveScore = new HashMap<Integer, Integer>();
        System.out.println("Score: " + id(8, turn) + " (" + evaluateCount + " evaluations)");
        return new Move(transposition.get(board.currentHash(turn))[2]);
    }

    protected int id(int depth, int turn) {
        int best = -INFINITY;
        for (int d = 1; d <= depth; ++d) {
            System.out.print("Depth: " + d);
            best = minimax(d, turn, -INFINITY, INFINITY);
            System.out.println(", value: " + best);
        }
        return best;
    }

    protected int minimax(int depth, int turn, int alpha, int beta) {
        if (depth == 0)
            return evaluate(turn);
        
        long hash = board.currentHash(turn);
        int[] history = transposition.get(hash);
        if (history != null && history[0] >= depth)
            return history[1];

        ++evaluateCount;
        transposition.put(hash, new int[] {depth, 0, 0});

        List<Integer> moves = board.generateMoves(turn);
        //Collections.sort(moves, compare);
        int bestMove = 0;

        for (int move: moves) {
            board.move(move);
            int t = -minimax(depth - 1, 1 - turn, -beta, -alpha);
            board.unmove();
            saveMoveScore(move, t);
            if (t > alpha) {
                alpha = t;
                bestMove = move;
                if (beta <= alpha)
                    return beta;
            }
        }

        transposition.put(board.currentHash(turn), new int[] {depth, alpha, bestMove});
        return alpha;
    }

    protected void saveMoveScore(int move, int score) {
        Integer current = moveScore.get(move);
        if (current == null || current.intValue() < score)
            moveScore.put(move, score);
    }

    protected int evaluate(int turn) {
        return board.staticValue(turn);
    }
}
