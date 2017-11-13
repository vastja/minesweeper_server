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
    this->fieldsToPlay = i * j;
    this->minesCount = minesCount;

    board = new int[i * j];
    revealed = new bool[i * j];

    initBoard();
}

Game::~Game() {
    delete[] board;
    delete[] revealed;
    board = NULL;
}

void Game::initBoard() {

    placeMines();

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
void Game::placeMines() {

    srand(time(NULL));
    int iPos, jPos;

    int count = minesCount; 
    while (count > 0) {
        iPos = ((double) rand() / (RAND_MAX)) * width;
        jPos = ((double) rand() / (RAND_MAX)) * height;

        if (checkRange(iPos, jPos, width, height) && board[iPos * width  + jPos] != MINE) {
            board[iPos * width + jPos] = MINE;
            count--;
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

    if (!revealed[i * width + j]) {
        fieldsToPlay--;
    }

    revealed[i * width + j] = true;
    return board[i * width + j];
}

bool Game::areFieldsToPlay() {
    return fieldsToPlay > this->minesCount;
}

bool Game::isRevealed(int i, int j) {

    return revealed[i * width + j];

}

int Game::getOponent(int playerId) {
    return playerId == player1Id ? player2Id : player1Id;
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
    
