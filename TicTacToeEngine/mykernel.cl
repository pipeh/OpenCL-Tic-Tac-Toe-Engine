kernel void vecadd( global int* A, global int* B, global int* C ) {
    const int idx = get_global_id(0);
    C[idx] = A[idx] + B[idx];
}

const __constant int lines[8][3] = {
        {0, 1, 2},
        {3, 4, 5}, 
        {6, 7, 8}, 
        {0, 3, 6}, 
        {1, 4, 7}, 
        {2, 5, 8}, 
        {0, 4, 8}, 
        {2, 4, 6}
        };

const __constant int scores[4] = {0, 1, 10, 100};

kernel void leafCalculation(global int* Pl, global int* V, global int pindex, global int eindex) {
    const int idx = get_global_id(0);
    int board[] = Pl[idx];
    int accum = 0;

    for(int l[] : lines) {
        // Recorrer el tablero y asignar puntajes
        int score = scores[(board[l[0]] == pindex) + (board[l[1]] == pindex) + (board[l[2]] == pindex)];
        score -= scores[(board[l[0]] == eindex) + (board[l[1]] == eindex) + (board[l[2]] == eindex)];
        accum += score; 
    }
    V[idx] = accum;
}

kernel void branchCalculation(global int Pl[][], global int Ml[][][], global int pindex, global int eindex) {
  const int idx = get_global_id(0); // 1
  int board[] = Pl[idx]; //2
  int generatedMoves[][];

  for (int i = 0; i < 9; i++) {
    if (board[i] == 0) {
      int new_move[9];
      std::copy(board, board + 9, new_move);
      new_move[i] = pindex;
      std::copy(new_move, new_move + 9, generatedMoves[i]);
    }
  }
  Ml[idx] = generatedMoves;
}