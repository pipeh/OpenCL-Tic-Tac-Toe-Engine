__constant int scores[4] = {0, 1, 10, 100};

__constant int lines[8][3] = {
        {0, 1, 2},
        {3, 4, 5}, 
        {6, 7, 8}, 
        {0, 3, 6}, 
        {1, 4, 7}, 
        {2, 5, 8}, 
        {0, 4, 8}, 
        {2, 4, 6}
        };

kernel void leafCalculation(global int* Pl, global int* V, global int *pindex, global int *eindex) {

    const int idx = get_global_id(0);
    const int k = idx * 9;

    // Obtener el tablero de juego correspondiente al thread
    int board[9];
    for (int i = 0; i < 9; i++) {
        board[i] = Pl[k + i];
    }

    // Recorrer el tablero y asignar puntajes
    int accum = 0;
    for(int i = 0; i < 8; i++) {
        __constant int* l = lines[i];
        int score = scores[(board[l[0]] == *pindex) + (board[l[1]] == *pindex) + (board[l[2]] == *pindex)];
        score -= scores[(board[l[0]] == *eindex) + (board[l[1]] == *eindex) + (board[l[2]] == *eindex)];
        accum += score; 
    }
    V[idx] = accum;
}

kernel void branchCalculation(global int* Pl, global int* Ml, global bool* B, global int *pindex, global int *moves) {
    const int idx = get_global_id(0);
    const int k = idx * 9;

    int board[9];

    for (int i = 0; i < 9; i++) {
        board[i] = Pl[k + i];
    }

    for (int i = 0; i < *moves; i++) {
        B[idx * *moves + i] = *moves == 1;
    }

    int generatedMoves[81];
    int count = 0;

    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            for (int j = 0; j < 9; j++) {
                generatedMoves[count * 9 + j] = board[j];
            }
            generatedMoves[count * 9 + i] = *pindex;

            for (int j = 0; j < 8; j++) {
                __constant int* l = lines[j];
                if (generatedMoves[count * 9 + l[0]] == *pindex && generatedMoves[count * 9 + l[1]] == *pindex &&
                generatedMoves[count * 9 + l[2]] == *pindex) {
                    B[idx * *moves + count] = true;
                    break;
                }
            }
            count++;
        }
    }

    for (int i = 0; i < 9 * *moves; i++) {
        Ml[idx * *moves * 9 + i] = generatedMoves[i];
    }
}