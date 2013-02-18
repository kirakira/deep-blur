package xiangqi;

import xiangqi.ai.*;
import java.util.*;
import java.io.*;

public class Main {
    public static final void main(String[] args) throws java.io.IOException {
        System.out.print("Transposition depth: ");
        Scanner scan = new Scanner(System.in);
        Agent agent = new Agent(scan.nextInt());
        agent.board.print();

        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        do {
            try {
                System.out.print("t to show whose turn, m to move, u to unmove, g to show all moves, a to show all attacks, s to search, e to evaluate, b to show cached best response, c to clear transposition, p to pass: ");
                String line = reader.readLine();
                String[] parts = line.split(" ");
                if (parts[0].equals("q"))
                    break;
                else if (parts[0].equals("m")) {
                    int x = Integer.parseInt(parts[1]), y = Integer.parseInt(parts[2]);
                    if (agent.checkedMove(new Move(x / 10, x % 10, y / 10, y % 10)))
                        agent.board.print();
                    else
                        System.out.println("Move failed");
                } else if (parts[0].equals("u")) {
                    agent.unmove();
                    agent.board.print();
                } else if (parts[0].equals("g") || parts[0].equals("a")) {
                    int turn = Integer.parseInt(parts[1]);
                    List<Integer> moves;
                    if (parts[0].equals("g"))
                        moves = agent.board.generateMoves(turn);
                    else
                        moves = agent.board.generateAttacks(turn);
                    System.out.print("" + moves.size() + " possible moves for player " + turn + ": ");
                    for (int m: moves)
                        System.out.print(new Move(m) + " ");
                    System.out.println();
                } else if (parts[0].equals("s")) {
                    Move move;
                    if (parts.length == 1)
                        move = agent.search(0, 10);
                    else
                        move = agent.search(Integer.parseInt(parts[1]), 0);
                    agent.checkedMove(move);
                    agent.board.print();
                } else if (parts[0].equals("e")) {
                    System.out.println(agent.evaluate());
                } else if (parts[0].equals("b")) {
                    agent.bestResponse();
                } else if (parts[0].equals("t")) {
                    System.out.println(agent.turn());
                } else if (parts[0].equals("c")) {
                    agent.clearTransposition();
                } else if (parts[0].equals("p")) {
                    agent.pass();
                    System.out.println(agent.turn());
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        } while (true);
    }
}
