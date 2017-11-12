#include <iostream>
#include <time.h>
#include <stdlib.h>

#include "Utils.hpp"
#include "Game.hpp"

Game::Game() {}

Game::Game(int player1Id, int player2Id, int i , int j, int minesCount) {

    this->player1Id = player1Id;
    this->player2Id = player2Id;
    this->width = i;
    this->height = j;

    board = new int[i * j];
    revealed = new bool[i * j];

    initBoard(minesCount);
}

Game::~Game() {
    delete[] board;
    delete[] revealed;
    board = NULL;
}

void Game::initBoard(int minesCount) {

    placeMines(minesCount);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            revealed[i * width + j] = false;
            if (board[i * width + j] != -1) {
                board[i * width + j] = findCountOfSurroundingMines(i, j);
            }
        }
    }
}


// TODO - divide board into areas and then place mines randomly
void Game::placeMines(int minesCount) {

    srand(time(NULL));
    int iPos, jPos;

    while (minesCount > 0) {
        iPos = ((double) rand() / (RAND_MAX)) * width;
        jPos = ((double) rand() / (RAND_MAX)) * height;

        if (checkRange(iPos, jPos, width, height) && board[iPos * width  + jPos] != MINE) {
            board[iPos * width + jPos] = MINE;
            minesCount--;
        }
    }
}

int Game::findCountOfSurroundingMines(int iPos, int jPos) {

    int count = 0;

    for (int i = iPos - 1; i <= iPos + 1; i++) {
        for (int j = jPos - 1; j <= jPos + 1; j++) {
            if (checkRange(i, j, width, height) && board[i * width + j] == MINE) {
                count++;
            }
        }
    }

    return count;
}

int Game::reveal(int i, int j) {

    revealed[i * width + j] = true;
    return board[i * width + j];
}

bool Game::isRevealed(int i, int j) {

    return revealed[i * width + j];

}

void Game::printBoard() {

    for (int i = 0; i < height; i++) {

        std::cout << "-----";
        for (int j = 0; j < width - 1; j ++) {
            std::cout << "----";
        }
        std::cout<<std::endl;
        std::cout << "|";
        for (int j = 0; j < width; j++) {

            if (board[i * width + j] >= 0) {
                std::cout << " "<< board[i * width + j] << " |";
            }
            else {
                std::cout << board[i * width + j] << " |";
            }
        }
        std::cout << std::endl;
    }

    std::cout << "-----";
    for (int j = 0; j < width - 1; j ++) {
        std::cout << "----";
    }
    std::cout<<std::endl;
}
    
