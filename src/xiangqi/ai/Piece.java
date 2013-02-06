package xiangqi.ai;

public class Piece {
    public static final int GENERAL = 1, G = 1;
    public static final int ADVISOR = 2, A = 2;
    public static final int ELEPHANT = 3, E = 3;
    public static final int HORSE = 4, H = 4;
    public static final int CHARIOT = 5, R = 5;
    public static final int CANNON = 6, C = 6;
    public static final int SOLDIER = 7, S = 7;

    public static final int GENERAL_RED = G, G_R = G;
    public static final int GENERAL_BLACK = 0x10 | G, G_B = 0x10 | G;
    public static final int ADVISOR_RED = A, A_R = A;
    public static final int ADVISOR_BLACK = 0x10 | A, A_B = 0x10 | A;
    public static final int ELEPHANT_RED = E, E_R = E;
    public static final int ELEPHANT_BLACK = 0x10 | E, E_B = 0x10 | E;
    public static final int HORSE_RED = H, H_R = H;
    public static final int HORSE_BLACK = 0x10 | H, H_B = 0x10 | H;
    public static final int CHARIOT_RED = R, R_R = R;
    public static final int CHARIOT_BLACK = 0x10 | R, R_B = 0x10 | R;
    public static final int CANNON_RED = C, C_R = C;
    public static final int CANNON_BLACK = 0x10 | C, C_B = 0x10 | C;
    public static final int SOLDIER_RED = S, S_R = S;
    public static final int SOLDIER_BLACK = 0x10 | S, S_B = 0x10 | S;
}
