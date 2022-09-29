#ifndef SOLVER_H
#define SOLVER_H

void readSudoku(int puzzle[][9]);
int checkValid (int puzzle[][9], int row, int col, int num);
void printSudoku(int puzzle[][9]);
int singleCandidate(int puzzle[][9], int row, int col);
void bruteForce(int puzzle[][9], int *attempts, int *backTracks);
int isSolved(int puzzle[][9]);

#endif