package xiangqi.ai;

import java.util.Stack;
import java.util.Random;
import java.util.List;
import java.util.ArrayList;

public class Board {
    public static final int W = 9, H = 10;

    protected int[][] board;
    protected int[] pieces;

    protected long[][][] hash;
    protected long[] playerHash;
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
        int[] count = new int[14];
        int[] startIndex = new int[]{0, 1, 3, 5, 7, 9, 11};

        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                if (board[i][j] != 0) {
                    int who = board[i][j] >> 4;
                    int piece = (board[i][j] & 0xf) - 1;
                    int index = startIndex[piece] + count[who * 7 + piece] + who * 16;
                    board[i][j] |= (index << 8);
                    pieces[index] = (i << 12) | (j << 8) | (board[i][j] & 0xff);
                    ++count[who * 7 + piece];
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

        playerHash = new long[] {random.nextLong(), random.nextLong()};
    }

    // returns true if the move caused the general to be captured
    public boolean move(int move) {
        boolean ret = false;

        int src_i = move >> 12, src_j = (move >> 8) & 0xf,
            dst_i = (move >> 4) & 0xf, dst_j = move & 0xf;

        int src = board[src_i][src_j], dst = board[dst_i][dst_j];
        if (dst != 0) {
            pieces[dst >> 8] = 0;
            currentHash ^= hash[dst_i][dst_j][dst & 0xff];

            if ((dst & 0xf) == Piece.G)
                ret = true;
        }

        board[dst_i][dst_j] = src;
        currentHash ^= hash[dst_i][dst_j][src & 0xff];

        board[src_i][src_j] = 0;
        currentHash ^= hash[src_i][src_j][src & 0xff];
        pieces[src >> 8] = (dst_i << 12) | (dst_j << 8) | (src & 0xff);

        history.push((dst << 16) | move);

        return ret;
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

    public long currentHash(int turn) {
        return currentHash ^ playerHash[turn];
    }

    public void print() {
        System.out.print("\t ");
        for (int j = 0; j < W; ++j)
            System.out.print(j + "   ");
        System.out.println();
        System.out.println();

        for (int i = 0; i < H; ++i) {
            if (i == 5) {
                for (int k = 0; k < 8; ++k)
                    System.out.print(' ');
                for (int k = 0; k < 34; ++k)
                    System.out.print('=');
                System.out.println();
            }
            System.out.print(i + "\t");
            for (int j = 0; j < W; ++j) {
                if (board[i][j] == 0)
                    System.out.print(" .  ");
                else {
                    if ((board[i][j] & 0x10) != 0)
                        System.out.print('~');
                    else
                        System.out.print(' ');
                    char c = 0;
                    switch (board[i][j] & 0xf) {
                        case Piece.G:
                            c = 'G';
                            break;

                        case Piece.A:
                            c = 'A';
                            break;

                        case Piece.E:
                            c = 'E';
                            break;

                        case Piece.H:
                            c = 'H';
                            break;

                        case Piece.R:
                            c = 'R';
                            break;

                        case Piece.C:
                            c = 'C';
                            break;

                        case Piece.S:
                            c = 'S';
                            break;
                    }
                    System.out.print(c + "  ");
                }
            }
            System.out.println();
        }
        System.out.println("Board hash: " + currentHash());
        System.out.println();
        System.out.println();
    }

    protected static int makePosition(int i, int j) {
        return (i << 4) | j;
    }

    protected static int[][][] gMove, aMove, eMove, eCheck, hMove, hCheck;
    static {
        gMove = new int[256][][];
        gMove[makePosition(0, 3)] = new int[][] {{0, 4}, {1, 3}};
        gMove[makePosition(0, 4)] = new int[][] {{0, 5}, {1, 4}, {0, 3}};
        gMove[makePosition(0, 5)] = new int[][] {{1, 5}, {0, 4}};
        gMove[makePosition(1, 3)] = new int[][] {{1, 4}, {2, 3}, {0, 3}};
        gMove[makePosition(1, 4)] = new int[][] {{1, 5}, {2, 4}, {1, 3}, {0, 4}};
        gMove[makePosition(1, 5)] = new int[][] {{2, 5}, {1, 4}, {0, 5}};
        gMove[makePosition(2, 3)] = new int[][] {{2, 4}, {1, 3}};
        gMove[makePosition(2, 4)] = new int[][] {{2, 5}, {2, 3}, {1, 4}};
        gMove[makePosition(2, 5)] = new int[][] {{2, 4}, {1, 5}};
        for (int i = 7; i <= 9; ++i)
            for (int j = 3; j <= 5; ++j) {
                int p = makePosition(i, j), op = makePosition(9 - i, j);;
                gMove[p] = new int[gMove[op].length][2];
                for (int k = 0; k < gMove[p].length; ++k) {
                    gMove[p][k][0] = 9 - gMove[op][k][0];
                    gMove[p][k][1] = gMove[op][k][1];
                }
            }

        aMove = new int[256][][];
        aMove[makePosition(0, 3)] = new int[][] {{1, 4}};
        aMove[makePosition(0, 5)] = new int[][] {{1, 4}};
        aMove[makePosition(1, 4)] = new int[][] {{0, 3}, {0, 5}, {2, 3}, {2, 5}};
        aMove[makePosition(2, 3)] = new int[][] {{1, 4}};
        aMove[makePosition(2, 5)] = new int[][] {{1, 4}};
        aMove[makePosition(9, 3)] = new int[][] {{8, 4}};
        aMove[makePosition(9, 5)] = new int[][] {{8, 4}};
        aMove[makePosition(8, 4)] = new int[][] {{9, 3}, {9, 5}, {7, 3}, {7, 5}};
        aMove[makePosition(7, 3)] = new int[][] {{8, 4}};
        aMove[makePosition(7, 5)] = new int[][] {{8, 4}};

        eMove = new int[256][][];
        eCheck = new int[256][][];
        eMove[makePosition(0, 2)] = new int[][] {{2, 0}, {2, 4}};
        eCheck[makePosition(0, 2)] = new int[][] {{1, 1}, {1, 3}};
        eMove[makePosition(0, 6)] = new int[][] {{2, 4}, {2, 8}};
        eCheck[makePosition(0, 6)] = new int[][] {{1, 5}, {1, 7}};
        eMove[makePosition(2, 0)] = new int[][] {{0, 2}, {4, 2}};
        eCheck[makePosition(2, 0)] = new int[][] {{1, 1}, {3, 1}};
        eMove[makePosition(2, 4)] = new int[][] {{0, 6}, {4, 6}, {4, 2}, {0, 2}};
        eCheck[makePosition(2, 4)] = new int[][] {{1, 5}, {3, 5}, {3, 3}, {1, 3}};
        eMove[makePosition(2, 8)] = new int[][] {{0, 6}, {4, 6}};
        eCheck[makePosition(2, 8)] = new int[][] {{1, 7}, {3, 7}};
        eMove[makePosition(4, 2)] = new int[][] {{2, 0}, {2, 4}};
        eCheck[makePosition(4, 2)] = new int[][] {{3, 1}, {3, 3}};
        eMove[makePosition(4, 6)] = new int[][] {{2, 4}, {2, 8}};
        eCheck[makePosition(4, 6)] = new int[][] {{3, 5}, {3, 7}};
        for (int i = 5; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                int p = makePosition(i, j), op = makePosition(9 - i, j);
                if (eMove[op] != null) {
                    eMove[p] = new int[eMove[op].length][2];
                    eCheck[p] = new int[eCheck[op].length][2];
                    for (int k = 0; k < eMove[p].length; ++k) {
                        eMove[p][k][0] = 9 - eMove[op][k][0];
                        eMove[p][k][1] = eMove[op][k][1];
                        eCheck[p][k][0] = 9 - eCheck[op][k][0];
                        eCheck[p][k][1] = eCheck[op][k][1];
                    }
                }
            }

        hMove = new int[256][][];
        hCheck = new int[256][][];
        int[] di = new int[] {-2, -1, 1, 2, 2, 1, -1, -2},
            dj = new int[] {1, 2, 2, 1, -1, -2, -2, -1},
            check_di = new int[] {-1, 0, 0, 1, 1, 0, 0, -1},
            check_dj = new int[] {0, 1, 1, 0, 0, -1, -1, 0};
        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                int count = 0;
                for (int r = 0; r < 8; ++r) {
                    int oi = i + di[r], oj = j + dj[r];
                    if (oi >= 0 && oi < H && oj >= 0 && oj < W)
                        ++count;
                }
                int p = makePosition(i, j);
                hMove[p] = new int[count][2];
                hCheck[p] = new int[count][2];
                count = 0;
                for (int r = 0; r < 8; ++r) {
                    int oi = i + di[r], oj = j + dj[r];
                    if (oi >= 0 && oi < H && oj >= 0 && oj < W) {
                        hMove[p][count][0] = oi;
                        hMove[p][count][1] = oj;
                        hCheck[p][count][0] = i + check_di[r];
                        hCheck[p][count][1] = j + check_dj[r];
                        ++count;
                    }
                }
            }
    }

    protected static int makeMove(int piece, int dst_i, int dst_j) {
        return (piece & 0xff00) | (dst_i << 4) | dst_j;
    }

    protected boolean checkPosition(int mask, int test, int i, int j) {
        int dst = board[i][j];
        return dst == 0 || ((dst & mask) ^ test) != 0;
    }

    public List<Integer> generateMoves(int turn) {
        ArrayList<Integer> ret = new ArrayList<Integer>(64);

        int start = turn << 4;
        int mask = 0xf0, test = turn << 4;

        // GENERAL
        if (pieces[start] != 0) {
            int p = pieces[start] >> 8;
            for (int k = 0; k < gMove[p].length; ++k)
                if (checkPosition(mask, test, gMove[p][k][0], gMove[p][k][1]))
                    ret.add(makeMove(pieces[start], gMove[p][k][0], gMove[p][k][1]));

            // check for generals in a column
            int altIndex = (1 - turn) << 4;
            int op = pieces[altIndex] >> 8;
            if (((op ^ p) & 0xf) == 0) {
                int from, to, j = p & 0xf;
                if (op < p) {
                    from = op >> 4;
                    to = p >> 4;
                } else {
                    from = p >> 4;
                    to = op >> 4;
                }
                boolean fail = false;
                for (int i = from + 1; i < to; ++i)
                    if (board[i][j] != 0) {
                        fail = true;
                        break;
                    }
                if (!fail)
                    ret.add((p << 8) | op);
            }
        }


        // ADVISOR
        for (int index = start + 1; index <= start + 2; ++index) {
            if (pieces[index] == 0)
                continue;
            int p = pieces[index] >> 8;
            for (int k = 0; k < aMove[p].length; ++k)
                if (checkPosition(mask, test, aMove[p][k][0], aMove[p][k][1]))
                    ret.add(makeMove(pieces[index], aMove[p][k][0], aMove[p][k][1]));
        }


        // ELEPHANT
        for (int index = start + 3; index <= start + 4; ++index) {
            if (pieces[index] == 0)
                continue;
            int p = pieces[index] >> 8;
            for (int k = 0; k < eMove[p].length; ++k)
                if (checkPosition(mask, test, eMove[p][k][0], eMove[p][k][1])
                        && board[eCheck[p][k][0]][eCheck[p][k][1]] == 0)
                    ret.add(makeMove(pieces[index], eMove[p][k][0], eMove[p][k][1]));
        }


        // HORSE
        for (int index = start + 5; index <= start + 6; ++index) {
            if (pieces[index] == 0)
                continue;
            int p = pieces[index] >> 8;
            for (int k = 0; k < hMove[p].length; ++k)
                if (checkPosition(mask, test, hMove[p][k][0], hMove[p][k][1])
                        && board[hCheck[p][k][0]][hCheck[p][k][1]] == 0)
                    ret.add(makeMove(pieces[index], hMove[p][k][0], hMove[p][k][1]));
        }

        return ret;
    }
}
