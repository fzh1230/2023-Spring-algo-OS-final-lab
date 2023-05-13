#include "generate.h"
#include<bits/stdc++.h>
#include<time.h>
#include "solve.h"
#include "generate.h"

using namespace std;


void initialiseSudoku(int grid[9][9]){
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            grid[i][j] = 0;
        }
    }
}



void newgameinit(int grid[9][9], int arr[], int level){

    srand(time(0));

    initialiseSudoku(grid);


    grid[0][0] = ((int)rand() % 9) + 1;
    grid[4][4] = ((int)rand() % 9) + 1;
    grid[8][8] = ((int)rand() % 9) + 1;

    if(solve(grid, arr)){
        for(int row = 0; row < 9; row++){
            for(int col = 0; col < 9; col++){
                if(level == 1){
                    if((int)rand() % 4 == 0){
                        grid[row][col] = 0;
                    }
                }else if(level == 2){
                   if((int)rand() % 3 == 0){
                        grid[row][col] = 0;
                    }
                }else{
                    if((int)rand() % 2 == 0){
                        grid[row][col] = 0;
                    }
                }
            }
        }
    }
}
