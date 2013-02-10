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
                System.out.print("m src dst to move, u to unmove, g player to show all moves for player: ");
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
                } else if (parts[0].equals("g")) {
                    int turn = Integer.parseInt(parts[1]);
                    List<Integer> moves = board.generateMoves(turn);
                    System.out.print("Possible moves: ");
                    for (Integer mi: moves) {
                        int m = mi.intValue();
                        System.out.print(parse(m >> 8) + "->" + parse(m & 0xff) + " ");
                    }
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
