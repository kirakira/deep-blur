package xiangqi.ai;

import java.util.Stack;

public class Board {
    public static final int W = 9, H = 10;

    protected int[][] board;
    protected int[] pieces;

    protected Stack<Integer> history = new Stack<Integer>();

    public Board(int[][] board, int turn) {
        this.board = board;
        initPieces();
    }

    public Board() {
        board = new int[][] {
            { 5,  4,  3,  2,  1,  2,  3,  4,  5},
            { 0,  0,  0,  0,  0,  0,  0,  0,  0},
            { 0,  6,  0,  0,  0,  0,  0,  6,  0},
            { 7,  0,  7,  0,  7,  0,  7,  0,  7},
            { 0,  0,  0,  0,  0,  0,  0,  0,  0},

            { 0,  0,  0,  0,  0,  0,  0,  0,  0},
            {23,  0, 23,  0, 23,  0, 23,  0, 23},
            { 0, 22,  0,  0,  0,  0,  0, 22,  0},
            { 0,  0,  0,  0,  0,  0,  0,  0,  0},
            {21, 20, 19, 18, 17, 18, 19, 20, 21}
        };
        initPieces();
    }

    protected void initPieces() {
        pieces = new int[32];
        int count = 0;

        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                if (board[i][j] != 0) {
                    board[i][j] |= (count << 8);
                    pieces[count] = (i << 12) | (j << 8) | (board[i][j] & 0xff);
                    ++count;
                }
            }
    }

    public void move(int move) {
        int src_i = move >> 12, src_j = (move >> 8) & 0xf,
            dst_i = (move >> 4) & 0xf, dst_j = move & 0xf;

        int src = board[src_i][src_j], dst = board[dst_i][dst_j];
        if (dst != 0)
            pieces[dst >> 8] = 0;

        board[dst_i][dst_j] = src;
        board[src_i][src_j] = 0;

        pieces[src >> 8] = (dst_i << 12) | (dst_j << 8) | (src & 0xff);

        history.push((dst << 16) | move);
    }
}
