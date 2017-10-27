#include <iostream>
#include <time.h>
#include <stdlib.h>

#include "Utils.hpp"

Game::Game(int player1Id, int player2Id, int i, int j, int minesCount) {

    this->player1Id = player1Id;
    this->player2Id = player2Id;

    board = new int[i][j];

    initBoard(board, i, j, minesCount);
}

Game::~Game() {
    delete board;
    board = NULL;
}

void Game::initBoard(int board[][], int width, int height, int minesCount) {

    placeMines(board, minesCount);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (board[i][j] != -1) {
                board[i][j] = findCountOfSurroundingMines(board, i, j, width, height);
            }
        }
    }
}


// TODO - divide board into areas and then place mines randomly
void Game::placeMines(int board[][], int i, int j, int minesCount) {

    srand(time(NULL));
    int iPos, jPos;

    while (minesCount > 0) {
        iPos = rand() * i;
        jPos = rand() * j;

        if (board[iPos][jPos] != MINE) {
            board[iPos][jPos] == MINE;
            minesCount--;
        }
    }
}

int Game::findCountOfSurroundingMines(int board[][], int iPos, int jPos, int width, int height) {

    int count++;

    for (int i = iPos - 1; i < iPos + 1; i++) {
        for (int j = jPos - 1; j < jPos + 1; j++) {
            if (checkrange(i, j, width, height) && board[i][j] == MINE) {
                count++;
            }
        }
    }

    return count;
}

