package xiangqi;

import xiangqi.ai.*;
import java.util.*;
import java.io.*;

public class Main {
    public static final void main(String[] args) throws java.io.IOException {
        Board board = new Board();
        board.print();

        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        do {
            try {
                System.out.print("m to move, u to unmove, g to show all moves, a to show all attacks: ");
                String line = reader.readLine();
                String[] parts = line.split(" ");
                if (parts[0].equals("q"))
                    break;
                else if (parts[0].equals("m")) {
                    int x = Integer.parseInt(parts[1]), y = Integer.parseInt(parts[2]);
                    board.move(((x / 10) << 12) | ((x % 10) << 8) | ((y / 10) << 4) | (y % 10));
                    board.print();
                } else if (parts[0].equals("u")) {
                    board.unmove();
                    board.print();
                } else if (parts[0].equals("g") || parts[0].equals("a")) {
                    int turn = Integer.parseInt(parts[1]);
                    List<Integer> moves;
                    if (parts[0].equals("g"))
                        moves = board.generateMoves(turn);
                    else
                        moves = board.generateAttacks(turn);
                    System.out.print("" + moves.size() + " possible moves for player " + turn + ": ");
                    for (int m: moves)
                        System.out.print(parse(m >> 8) + "->" + parse(m & 0xff) + " ");
                    System.out.println();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        } while (true);
    }

    public static String parse(int x) {
        return "(" + (x >> 4) + ", " + (x & 0xf) + ")";
    }
}
