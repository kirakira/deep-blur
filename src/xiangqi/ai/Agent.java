package xiangqi.ai;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.Comparator;
import java.util.Collections;

public class Agent {
    protected int turn;
    protected int[] score;
    protected static final int INFINITY = 10000, ABORTED = 20000;
    public Board board;

    // lower(16) upper(16) move(16) depth(16)
    protected Map<Long, Long> transposition = new HashMap<Long, Long>();
    protected Map<Integer, Integer> moveScore = new HashMap<Integer, Integer>(),
              currentMoveScore = new HashMap<Integer, Integer>();

    protected int checkTime = 0;
    protected static final int CHECK_TIME_CYCLE = 100000;

    protected Comparator<Integer> compare = new Comparator<Integer>() {
        public int compare(Integer o1, Integer o2) {
            if (o1 == 0)
                return -1;
            if (o2 == 0)
                return 1;

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
        score = new int[2];
    }

    public Agent(int[][] board, int turn) {
        this.board = new Board(board);
        this.turn = turn;
        score = new int[2];
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
        score = new int[2];
    }

    protected int evaluateCount = 0;

    public Move search() {
        evaluateCount = 0;
        long startTime = System.nanoTime();
        int move = id(0, turn, 10L * 1000000000L);
        long timeSpent = System.nanoTime() - startTime;
        System.out.println(evaluateCount + " evaluations in " + timeSpent / 1e9 + "s, " + (int) ((double) evaluateCount / (timeSpent / 1e6)) + " k/s");
        checkMemory();
        if (move != 0)
            return new Move(move);
        else
            return null;
    }

    protected void checkMemory() {
        Runtime runtime = Runtime.getRuntime();
        double ratio = (double) runtime.totalMemory() / (double) runtime.maxMemory();
        int limit = (int) ((double) runtime.maxMemory() * .0047);
        System.out.print(transposition.size() + " entries (limit: " + limit + "). Memory: " + ratio + " of " + runtime.maxMemory() / (1L << 20) + "MB. ");
        if (transposition.size() > limit) {
            System.out.print("Clearing transposition.. ");
            transposition.clear();
        }
        System.out.println();
    }

    protected int id(int depth, int turn, long timeLimit) {
        moveScore = new HashMap<Integer, Integer>();
        currentMoveScore = new HashMap<Integer, Integer>();
        int best = -INFINITY, bestMove = 0;

        long deadLine;
        if (timeLimit == 0)
            deadLine = Long.MAX_VALUE;
        else
            deadLine = System.nanoTime() + timeLimit;

        for (int d = 1; (depth == 0 ? true : (d <= depth)); ++d) {
            System.out.print("Depth: " + d);
            int t = MTDf(d, turn, score[turn], deadLine);
            if (t == ABORTED) {
                System.out.println(", aborted");
                break;
            }
            moveScore = currentMoveScore;
            currentMoveScore = new HashMap<Integer, Integer>();

            best = t;
            System.out.print(", value: " + best + ", move: ");

            t = turn;
            int i;
            for (i = 0; i < d; ++i) {
                Long iHistory = transposition.get(board.currentHash(t));
                if (iHistory == null)
                    break;
                long history = iHistory.longValue();
                int move = (int) (history >> 16) & 0xffff;
                if (i == 0)
                    bestMove = move;
                if (move == 0)
                    break;
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
        score[turn] = best;
        return bestMove;
    }

    protected int MTDf(int depth, int turn, int g, long deadLine) {
        int lower = -INFINITY, upper = INFINITY;
        while (lower < upper) {
            int beta;
            if (g == lower)
                beta = g + 1;
            else
                beta = g;
            g = minimax(depth, turn, beta - 1, beta, false, deadLine);
            if (g == ABORTED)
                return ABORTED;

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

    public int evaluate() {
        return minimax(0, turn, -INFINITY, INFINITY, false, 0);
    }

    protected int minimax(int depth, int turn, int alpha, int beta, boolean nullMove, long deadLine) {
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

        boolean quiescence = (depth == 0);

        ++evaluateCount;
        Long oldHistory = storeTransposition(hash, depth, 0, 0, 0);

        List<Integer> moves;
        if (quiescence) {
            moves = board.generateAttacks(turn);
            if (moves.size() == 0) {
                boolean checkmate = false;
                if (board.isChecked(turn)) {
                    checkmate = true;
                    moves = board.generateMoves(turn);
                    for (int move: moves)
                        if (board.move(move)) {
                            board.unmove();
                            checkmate = false;
                            break;
                        }
                }
                int score;
                if (checkmate)
                    score = -INFINITY;
                else
                    score = board.staticValue(turn);
                storeTransposition(hash, 0, 0, score, score);
                return score;
            }
        } else
            moves = board.generateMoves(turn);
        if (nullMove && moves.size() > 0 && !board.isChecked(turn))
            moves.add(0);
        Collections.sort(moves, compare);

        int best = -INFINITY, bestMove = 0, oldAlpha = alpha;
        boolean aborted = false;
        for (int move: moves) {
            ++checkTime;
            if (checkTime == CHECK_TIME_CYCLE) {
                checkTime = 0;
                if (System.nanoTime() >= deadLine) {
                    aborted = true;
                    break;
                }
            }

            if (move != 0 && !board.move(move))
                continue;

            int t;
            if (quiescence)
                t = -minimax(0, 1 - turn, -beta, -alpha, (move != 0), deadLine);
            else
                t = -minimax(depth - 1, 1 - turn, -beta, -alpha, (move != 0), deadLine);
            if (move != 0)
                board.unmove();

            if (t == -ABORTED) {
                aborted = true;
                break;
            }

            if (move != 0)
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
        
        if (aborted) {
            if (oldHistory != null)
                transposition.put(hash, oldHistory);
            else
                transposition.remove(hash);
            return ABORTED;
        }

        // bug note: we don't use static values if there's an beta cutoff
        if (quiescence && best < beta && bestMove == 0) {
            best = board.staticValue(turn);
            storeTransposition(hash, 0, 0, best, best);
        } else {
            if (best <= oldAlpha)
                storeTransposition(hash, depth, bestMove, best, -INFINITY);
            else if (best < beta)
                storeTransposition(hash, depth, bestMove, best, best);
            else
                storeTransposition(hash, depth, bestMove, INFINITY, best);
        }

        return best;
    }

    protected void saveMoveScore(int move, int score) {
        Integer current = currentMoveScore.get(move);
        if (current == null || current.intValue() < score)
            currentMoveScore.put(move, score);
    }
}
