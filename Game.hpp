#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <cstring>

#include "Timer.hpp"
#include "BaseTimer.hpp"

class Server; // forward decleration

class Game : public BaseTimer {
    
    private:

        Server * server;
        Timer * roundTimer;

        int width;
        int height;
        int *board;
        bool *revealed;

        int onTurn;
        bool inProgress;

        int player1Id;
        int player2Id;

        // TODO
        char *player1Name;
        char *player2Name;

        int fieldsToPlay;
        int minesCount;

        char * gameCode;
    
        void initBoard();
        void placeMines();
        
        int findCountOfSurroundingMines(int iPos, int jPos);

        int reveal(int i, int j);
        bool isThisGamePlayer(int playerId) {
            return (playerId == player1Id || playerId == player2Id);
        }
        void endTurn(int id);
        int doRevealCell(int i, int j);
    
    public:

        static const char REVEALED = -2;
        static const char MINE = -1;
        static const char DEATH = 0;
        static const char DRAW = 1;
        static const char TIMEOUT = 2;
        static const char SURRENDER = 3;

        void printBoard();
        void endGameReveal();
        void doReveal(int i, int j);
        void revealStateOfGame(int id);
        bool isRevealed(int i, int j);
        bool areFieldsToPlay();
        int getOponent(int playerId);
        void setTurn(int id);
       
        void endGame(int id);
        int getTurn();

        virtual void onAction();

        void getPlayers(int * player1Id, int * player2Id) { *player1Id = this->player1Id; *player2Id = this->player2Id;};
        bool reconnect(int id, char code[]);
        
        Game(){};
        Game(Server * server, int player1Id, int player2Id, int i = 10, int j = 10, int minesCount = 10);
        ~Game();
};

#endif