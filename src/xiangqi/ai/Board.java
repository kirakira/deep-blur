package xiangqi.ai;

import java.util.Stack;
import java.util.Random;

public class Board {
    public static final int W = 9, H = 10;

    protected int[][] board;
    protected int[] pieces;

    protected long[][][] hash;
    protected long currentHash;

    protected Stack<Integer> history = new Stack<Integer>();

    public Board(int[][] board, int turn) {
        this.board = board;
        initPieces();
        initHasher();
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
        initHasher();
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

    protected void initHasher() {
        hash = new long[H][W][32];
        Random random = new Random(0);
        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j)
                for (int k = 0; k < hash[i][j].length; ++k)
                    hash[i][j][k] = random.nextLong();

        currentHash = 0;
        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j)
                if (board[i][j] != 0)
                    currentHash ^= hash[i][j][board[i][j] & 0xff];
    }

    public void move(int move) {
        int src_i = move >> 12, src_j = (move >> 8) & 0xf,
            dst_i = (move >> 4) & 0xf, dst_j = move & 0xf;

        int src = board[src_i][src_j], dst = board[dst_i][dst_j];
        if (dst != 0) {
            pieces[dst >> 8] = 0;
            currentHash ^= hash[dst_i][dst_j][dst & 0xff];
        }

        board[dst_i][dst_j] = src;
        currentHash ^= hash[dst_i][dst_j][src & 0xff];

        board[src_i][src_j] = 0;
        currentHash ^= hash[src_i][src_j][src & 0xff];
        pieces[src >> 8] = (dst_i << 12) | (dst_j << 8) | (src & 0xff);

        history.push((dst << 16) | move);
    }

    public void unmove() {
        int last = history.pop();
        int capture = last >> 16, move = last & 0xffff;

        int src_i = move >> 12, src_j = (move >> 8) & 0xf,
            dst_i = (move >> 4) & 0xf, dst_j = move & 0xf;
        int dst = board[dst_i][dst_j];
        board[src_i][src_j] = dst;
        currentHash ^= hash[src_i][src_j][dst & 0xff];
        pieces[dst >> 8] = (src_i << 12) | (src_j << 8) | (dst & 0xff);

        board[dst_i][dst_j] = capture;
        currentHash ^= hash[dst_i][dst_j][dst & 0xff];
        if (capture != 0) {
            pieces[capture >> 8] = (dst_i << 12) | (dst_j << 8) | (capture & 0xff);
            currentHash ^= hash[dst_i][dst_j][capture & 0xff];
        }
    }

    public long currentHash() {
        return currentHash;
    }

    public void print() {
        for (int i = 0; i < H; ++i) {
            for (int j = 0; j < W; ++j) {
                if (board[i][j] == 0)
                    System.out.print(".");
                else
                    System.out.print(board[i][j] & 0xf);
                System.out.print(" ");
            }
            System.out.println();
        }
        System.out.println("Board hash: " + currentHash());
        System.out.println();
    }
}
