package xiangqi.ai;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.Comparator;
import java.util.Collections;

public class Agent {
    protected int turn;
    protected static final int INFINITY = 10000;
    public Board board;

    // lower(16) upper(16) move(16) depth(16)
    protected Map<Long, Long> transposition = new HashMap<Long, Long>();
    protected Map<Integer, Integer> moveScore, currentMoveScore;

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
        long startTime = System.nanoTime();
        int score = id(6, turn);
        long timeSpent = System.nanoTime() - startTime;
        System.out.println("Score: " + score + " (" + evaluateCount + " evaluations in " + timeSpent / 1e9 + "s, " + (int) ((double) evaluateCount / (timeSpent / 1e6)) + " k/s)");
        Long iHistory = transposition.get(board.currentHash(turn));
        checkMemory();
        if (iHistory != null) {
            long history = iHistory.longValue();
            int move = (int) ((history >> 16) & 0xffffL);
            int lower = (int) (history >> 48), upper = (int) (history << 16 >> 48);
            if (move != 0)
                return new Move(move);
            else
                return null;
        } else
            return null;
    }

    protected void checkMemory() {
        Runtime runtime = Runtime.getRuntime();
        double ratio = (double) runtime.totalMemory() / (double) runtime.maxMemory();
        System.out.print(transposition.size() + " entries. Memory: " + ratio + " of " + runtime.maxMemory() / (1L << 20) + "MB. ");
        if (ratio >= .7) {
            System.out.print("Running gc.. ");
            transposition = new HashMap<Long, Long>();
            runtime.gc();
        }
        System.out.println();
    }

    protected int id(int depth, int turn) {
        moveScore = new HashMap<Integer, Integer>();
        currentMoveScore = new HashMap<Integer, Integer>();
        int best = -INFINITY;
        for (int d = 1; d <= depth; ++d) {
            System.out.print("Depth: " + d);
            best = MTDf(d, turn);
            System.out.print(", value: " + best + ", move: ");

            moveScore = currentMoveScore;
            currentMoveScore = new HashMap<Integer, Integer>();

            int t = turn, i;
            for (i = 0; i < d; ++i) {
                Long iHistory = transposition.get(board.currentHash(t));
                if (iHistory == null)
                    break;
                long history = iHistory.longValue();
                int move = (int) (history >> 16) & 0xffff;
                board.move(move);
                t = 1 - t;
                if (i == 0)
                    System.out.print("0x" + Integer.toString(move, 16));
                System.out.print(new Move(move) + " ");
            }
            while (i > 0) {
                board.unmove();
                --i;
            }
            System.out.println();
        }
        return best;
    }

    protected int MTDf(int depth, int turn) {
        int lower = -INFINITY, upper = INFINITY;
        int g = 0;
        while (lower < upper) {
            int beta;
            if (g == lower)
                beta = g + 1;
            else
                beta = g;
            g = minimax(depth, turn, beta - 1, beta, 0);
            if (g < beta)
                upper = g;
            else
                lower = g;
        }
        // note: it is possible that lower > upper here
        // this is caused by having found an over-depth tranposition entry
        // thus the g here is more accureate than desired depth
        return g;
    }

    protected Long storeTransposition(long hash, int depth, int move, int upper, int lower) {
        return transposition.put(hash, ((((long) lower & 0xffffL) << 48)
                    | (((long) upper & 0xffffL) << 32) | ((long) move << 16) | (long) depth));
    }

    protected int minimax(int depth, int turn, int alpha, int beta, int debug) {
        long hash = board.currentHash(turn);
        if (transposition.containsKey(hash)) {
            long history = transposition.get(hash);
            if ((history & 0xffff) >= depth) {
                int lower = (int) (history >> 48), upper = (int) (history << 16 >> 48);
                if (lower == upper)
                    return lower;
                if (lower >= beta)
                    return lower;
                if (upper <= alpha)
                    return upper;
                alpha = Math.max(alpha, lower);
                beta = Math.min(beta, upper);
            }
        }

        if (depth == 0)
            return evaluate(turn);
        
        ++evaluateCount;
        Long oldHistory = storeTransposition(hash, depth, 0, 0, 0);

        List<Integer> moves = board.generateMoves(turn);
        Collections.sort(moves, compare);

        int best = -INFINITY, bestMove = 0, oldAlpha = alpha;
        for (int move: moves) {
            if (!board.move(move))
                continue;

            int t = -minimax(depth - 1, 1 - turn, -beta, -alpha, 0);
            board.unmove();
            saveMoveScore(move, t);

            if (t > best) {
                best = t;
                bestMove = move;
            }

            if (best > alpha) {
                alpha = best;
                if (beta <= alpha)
                    break;
            }
        }

        if (best <= oldAlpha)
            storeTransposition(hash, depth, bestMove, best, -INFINITY);
        else if (best < beta)
            storeTransposition(hash, depth, bestMove, best, best);
        else
            storeTransposition(hash, depth, bestMove, INFINITY, best);

        return best;
    }

    protected void saveMoveScore(int move, int score) {
        Integer current = currentMoveScore.get(move);
        if (current == null || current.intValue() < score)
            currentMoveScore.put(move, score);
    }

    protected int evaluate(int turn) {
        return board.staticValue(turn);
    }
}
