package xiangqi.ai;

import java.util.Stack;

public class Board {
    public static final int W = 9, H = 10;
    public static final int EMPTY = -1;

    protected Stack<Integer> history = new Stack<Integer>();

    protected int turn;
    protected int[][] board;
    protected int[][][] pieces = new int[2][Piece.typesCount][6];

    public Board(int[][] board, int turn) {
        this.board = board;
        this.turn = turn;
        initPieces();
    }

    public Board() {
        turn = 0;
        board = new int[][] {
            { 4,  3,  2,  1,  0,  1,  2,  3,  4},
            {-1, -1, -1, -1, -1, -1, -1, -1, -1},
            {-1,  5, -1, -1, -1, -1, -1,  5, -1},
            { 6, -1,  6, -1,  6, -1,  6, -1,  6},
            {-1, -1, -1, -1, -1, -1, -1, -1, -1},

            {-1, -1, -1, -1, -1, -1, -1, -1, -1},
            {22, -1, 22, -1, 22, -1, 22, -1, 22},
            {-1, 21, -1, -1, -1, -1, -1, 21, -1},
            {-1, -1, -1, -1, -1, -1, -1, -1, -1},
            {20, 19, 18, 17, 16, 17, 18, 19, 20}
        };
        initPieces();
    }

    protected void initPieces() {
        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                if (board[i][j] == EMPTY)
                    continue;
                int d1 = board[i][j] & 0xf0, d2 = board[i][j] & 0x0f;
                ++pieces[d1][d2][0];
                pieces[d1][d2][pieces[d1][d2][0]] = ((i << 8) | j);
            }
    }

    public void pass() {
        turn = 1 - turn;
    }
}
