package xiangqi.ai;

public class Move {
    protected int move;

    public Move(int srcRow, int srcCol, int dstRow, int dstCol) {
        move = (srcRow << 12) | (srcCol << 8) | (dstRow << 4) | (dstCol);
    }

    public Move(int move) {
        this.move = move;
    }

    public int internalRepresentation() {
        return move;
    }

    public int srcRow() {
        return move >> 12;
    }

    public int srcCol() {
        return (move >> 8) & 0xf;
    }

    public int dstRow() {
        return (move >> 4) & 0xf;
    }

    public int dstCol() {
        return move & 0xf;
    }
}
