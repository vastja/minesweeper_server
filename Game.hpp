#ifndef _GAME_HPP_
#define _GAME_HPP_

const int MINE = -1;

class Game {
    
    private:
    
        int *board;
        bool *revealed;
        int player1Id;
        int player2Id;
        
    
        void initBoard(int minesCount);
        void placeMines(int minesCount);
        int findCountOfSurroundingMines(int iPos, int jPos);
    
    public:
    
        int width;
        int height;

        void printBoard();

        int reveal(int i, int j);   
        bool isRevealed(int i, int j);
        Game();
        Game(int player1Id, int player2Id, int i = 10, int j = 10, int minesCount = 10);
        ~Game();
};

#endif