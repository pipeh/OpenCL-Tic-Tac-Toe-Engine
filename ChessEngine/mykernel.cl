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

kernel leafCalculation(global int* Pl) {
    const int idx = get_global_id(0);
    
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

kernel branchCalculation(global int* Pl) {
    const int idx = get_global_id(0); \\ 1
    \\2
    for()
    
}