#include <iostream>
#include <stdlib.h>

#include "Utils.hpp"
#include "Game.hpp"
#include "Server.hpp"

Game::Game(Server * server, int player1Id, int player2Id, int i , int j, int minesCount) : server(server) {

    this->player1Id = player1Id;
    this->player2Id = player2Id;
    this->width = i;
    this->height = j;
    this->fieldsToPlay = i * j;
    this->minesCount = minesCount;
    
    player1Away = false;
    player2Away = false;
    
    roundTimer = new Timer();

    board = new int[i * j];
    revealed = new bool[i * j];

    initBoard();

    inProgress = true;
    onTurn = -1;

    int len = 10 + (rand() % 10);
    gameCode = new char[len];
    generateGameCode(gameCode, len); 

    this->server->sendMessage(player1Id, START_GAME, gameCode);
    this->server->sendMessage(player2Id, START_GAME, gameCode);
}

Game::~Game() {

    roundTimer->cancel();

    delete [] board;
    delete [] revealed;
    delete [] gameCode;
    delete roundTimer;

    board = NULL;
    revealed = NULL;
    gameCode = NULL;
    roundTimer = NULL;
}

void Game::initBoard() {

      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            revealed[i * width + j] = false;
            board[i * width + j] = 0;
          
        }
    }

    placeMines();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
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

void Game::endTurn(int id) {
    if (onTurn == id && inProgress) {
        this->roundTimer->cancel();
    }
}

void Game::setTurn(int id) {

    if (onTurn == id || !(isThisGamePlayer(id))) {
        return;
    }

    endTurn(onTurn);
    onTurn = id;
    
    //TODO
    std::thread timer (&Timer::start, roundTimer, 30, this);
    timer.detach();

    inProgress = true;

    this->server->sendMessage(id, START_TURN, NULL);
}

int Game::getTurn() {
    return onTurn;
}

void Game::onAction() {
    this->inProgress = false;
    int oponentId = getOponent(onTurn);

    this->server->executeEndGameResponse(onTurn, oponentId, Game::TIMEOUT);
};

void Game::endGameReveal(int id) {

    int mines;

    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            
            mines = this->reveal(i, j);

            if (mines == MINE) {
                this->server->executeRevealResponse(id, i, j, MINE, true);
            }
        }
    }
}

int Game::doRevealCell(int i, int j) {

    if (isRevealed(i, j)) {
        return Game::REVEALED;
    }

    int mines = reveal(i, j);

    int oponentId = getOponent(onTurn);

    this->server->executeRevealResponse(onTurn, i, j, mines, false);
    this->server->executeRevealResponse(oponentId, i, j, mines, false);
   

    if (mines == MINE) {
        return MINE;
    }
    else if (mines == 0) {

        for (int w = i - 1; w <= i + 1; w++) {
            for (int h = j - 1; h <= j + 1; h++) {
                if (checkRange(w, h, width, height)) {
                    doRevealCell(w, h);
                }
            }
        }

    }

    return mines;
}

void Game::doReveal(int i, int j) {

    if (!checkRange(i, j, width, height)) {
        this->server->sendMessage(onTurn, REVEAL_REFUSED, REVEAL_E4);
        return;
    }

    int result = doRevealCell(i, j);

    int oponentId = getOponent(onTurn);
    if (result == Game::MINE) { 
        this->server->executeEndGameResponse(oponentId, onTurn, Game::DEATH);
        inProgress = false;
    } 
    else if (!areFieldsToPlay()) {
        this->server->executeEndGameResponse(oponentId, onTurn, Game::DRAW);
        inProgress = false;
    }
    else if (result == Game::REVEALED) {
        this->server->sendMessage(onTurn, REVEAL_REFUSED, REVEAL_E3);
    }
    else {
        setTurn(oponentId);
    }
}

void Game::endGame(int id) {
    int oponentId = getOponent(id);
    this->server->executeEndGameResponse(oponentId, id, Game::SURRENDER);
}

bool Game::reconnect(int id, int oldId, const char * code) {

    if ((oldId == player1Id || oldId == player2Id) && strcmp(code, gameCode) == 0) {

        if (id == player1Id) {
            player1Id = id; 
            player1Away = false;
        }
        else {
            player2Id = id; 
            player2Away = false;
        }

        this->server->sendMessage(id, RECONNECT, NULL);
        return true;
    }
    else {
        this->server->sendMessage(id, RECONNECT_REFUSED, RECONNECT_E1);
        return false;
    }

}

void Game::revealStateOfGame(int id) {

    if (onTurn == id) {
        this->server->sendMessage(id, START_TURN, NULL);
        onTurn = id;
    }

    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            if (isRevealed(i, j)) {
                this->server->executeRevealResponse(id, i, j, board[i * width + j], false);
            }
        }
    }

}