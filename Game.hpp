#ifndef _GAME_HPP_
#define _GAME_HPP_

class Game {
    
    private:
    
        int *board;
        int player1Id;
        int player2Id;
    
        void initBoard(int *board, int i, int j, int minesCount);
        void placeMines(int *board, int i, int j, int minesCount);
        int findCountOfSurroundingMines(int *board, int iPos, int jPos, int width, int height);
    
    public:
    
        int checkField(int i, int j);
        Game();
        Game(int player1Id, int player2Id, int i = 20, int j = 20, int minesCount = 10);
        ~Game();
};

#endif