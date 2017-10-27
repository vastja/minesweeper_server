class Game {
    
    private:
    
        int **board;
        const int player1Id;
        const int player2Id;
    
        const int MINE = -1;
    
        void initBoard(int board[][], int i, int j, int minesCount);
        void placeMines(int board[][], int i, int j, int minesCount);
        int findCountOfSurroundingMines(int board[][], int iPos, int jPos, int width, int height);
    
    public:
    
        int checkField(int i, int j);
        Game(int player1Id, int player2Id, int i, int j, int minesCount);
        ~Game();
    };