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

        int player1Id;
        int player2Id;

        bool player1Away;
        bool player2Away;

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

        static const int ROUND_TIME = 30;

        static const char REVEALED = -2;
        static const char MINE = -1;
        static const char DEATH = 0;
        static const char DRAW = 1;
        static const char TIMEOUT = 2;
        static const char SURRENDER = 3;
        static const char EXIT = 4;

        void printBoard();
        void endGameReveal(int id);
        void doReveal(int i, int j);
        void revealStateOfGame(int id);
        bool isRevealed(int i, int j);
        bool areFieldsToPlay();
        int getOponent(int playerId);
        void setTurn(int id);
       
        void endGame(int id);
        void endGameSurrender(int id);
        int getTurn();

        virtual void onAction();

        void getPlayers(int * player1Id, int * player2Id) { *player1Id = this->player1Id; *player2Id = this->player2Id;};
        bool reconnect(int id, int oldId, const char * code);

        void setAway(int id) {
            if (id == player1Id) {
                player1Away = true;
            }
            else if (id == player2Id) {
                player2Away = true;
            }
        };

        bool isAway(int id) {
            if (id == player1Id) {
                return player1Away;
            }
            else  if (id == player2Id) {
                return player2Away;
            }
        };

        Game(){};
        Game(Server * server, int player1Id, int player2Id, int i = 10, int j = 10, int minesCount = 10);
        ~Game();
};

#endif