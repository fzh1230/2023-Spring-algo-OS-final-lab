#ifndef SOLVE_H
#define SOLVE_H

    void findEmpty(int size[9][9]);

    bool isValid(int size[9][9], int row, int col, int num);

    bool solve(int size[9][9], int arr[]);

    void printGrid(int size[9][9]);

#endif // SOLVE_H
