kernel void vecadd( global int* A, global int* B, global int* C ) {
    const int idx = get_global_id(0);
    C[idx] = A[idx] + B[idx];
}

/*
Leaf:
Input: Pl : "a list of game positions"
Output: V
1: Get id of the current thread;
2: Restore position Pid for the current thread, according to id and Pl;
3: for each non-empty point p in Pid do
4: Recognize the shapes of the point p according to
game rules;
5: Set the score for the shape;
6: Accumulate the score for p;
7: end for
8: Add up all th

*/

kernel leafCalculation(global int** Pl) {
    const int idx = get_global_id(0);
    int* board = Pl[idx]; //2
    int accum = 0;
    for(int i = 0; i < 64; i++) {
        // Recorrer el tablero y asignar puntajes
        int score;
        accum += score; 
    }
    // 8
    // return V;
    
}

/*
Branch:
Input: Pl : "a list of game positions"
Output: Ml
1: Get id of the current thread;
2: Restore position Pid for the current thread, according to id and Pl;
3: for each legal point p in Pid do
4: Recognize the shapes of the point p according to
game rules;
5: Set the score for the shape;
6: Accumulate the score for p;
7: end for
8: Assemble the valuable moves as the move list Ml;
9: return Ml;
*/

// Pl es un array con todos los tableros extendidos [[0..63],[64..127]]
kernel branchCalculation(global int** Pl, global int** Ml) {
    const int idx = get_global_id(0); // 1
    int* board = Pl[idx]; //2
    int** Bl; // [[0..63],[64..127]]
    int accum = 0;

    for(int i = 0; i < 64; i++) {
        // Eva= board[i];luar si board[i] puede moverse
        // Calcular nuevos tableros para cada pieza (es necesario saber cómo se mueve cada pieza)
        // Quedarse con los mejores tableros
        int p = board[i];
        Bl[p] = getMoves(p); // Hay que definir esto
        //int score;
        //accum += score; 
    }
}

int** getMoves(int* b, char p, int pos) {
    int** boards;
    if (p == 'p') {
        if (pos + 8 <= 64) {
            int* b1 = copy(board);
            b1[pos] = 0;
            b1[pos + 8] = 1;
            boards[0] = b1;
        }
    } else if (p == 'r') {

    } else if (p == 'n') {

    } else if (p == 'b') {

    } else if (p == 'q') {

    } else if (p == 'k') {

    }

    return boards;
}