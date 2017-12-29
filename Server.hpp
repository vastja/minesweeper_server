#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <queue> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <semaphore.h>
#include <fstream>

/* Start game */
const char START_GAME = 4;
const char START_GAME_ACCEPTED = 5;
const char START_GAME_REFUSED = 6;
/* Reveal */
const char REVEAL = 7;
const char REVEAL_REFUSED = 8;
const char WIN = 9;
const char LOSE = 10;
const char DRAW = 11;
/* Surrender */
const char SURRENDER = 12;
const char SURRENDER_REFUSED = 13;
const char SURRENDER_WIN = 14;
const char SURRENDER_LOSE = 15;
/* Reconnect */
const char RECONNECT = 16;
const char RECONNECT_REFUSED = 17;
/* End game */
const char END_GAME = 18;
const char END_GAME_REFUSED = 19;
const char END_GAME_REVEAL = 20;
/* Support functions */
const char SEND_ID = 21;
const char START_TURN = 22;
const char TIMEOUT_WIN = 23;
const char TIMEOUT_LOSE = 24;
const char ALIVE = 25;
const char MESSAGE_BAD_FORMAT = 26;

const char START_GAME_E1[] = "Already in game";
const char START_GAME_E2[] = "Already waiting";
const char REVEAL_E1[] = "Not your turn";
const char REVEAL_E2[] = "Game not found";
const char REVEAL_E3[] = "Already revealed";
const char REVEAL_E4[] = "Game error";
const char SURRENDER_E1[] = "Game not found";
const char RECONNECT_E1[] = "Bad code or id";
const char RECONNECT_E2[] = "Game not found";
const char END_GAME_E1[] = "Game not found";
const char BAD_FORMAT_ERROR_E1[] = "Invalid message format";

const int CONNECTION_TIMEOUT = 10;
const int MAX_CLIENTS_LIMIT = 65500;

struct StatInfo {
    int clientsConnected = 0;
    int clientsDisconnected = 0;
    int clientsIdleDisconnected = 0;
    int recievedCount = 0;
    int sentCount = 0;
    int gamesStarted = 0;
    int notValidMessagesCount = 0;
    int notValidRequestsCount = 0;
};

class Game; // forward decleration
class Client;
class IdleChecker;

class Server {
    
    private:

        struct StatInfo info;

        bool run;

        sem_t semaphore;
        std::queue<int> idleQueue;       
    
        std::queue<int> waitingForGame;
        Game **games;
        Client **clients;
        IdleChecker * idleChecker;

        int maxClients;
        int serverQueueSize;
        int server_socket;

        struct sockaddr_in my_addr;
        
        char *cbuf;
        int bufferSize;
    
        fd_set client_socks;
        
        
        int stop();
        void execute(fd_set clientsSet, int fd);
        void read(int fd);
        void acceptNewClient(int fd);

        void executeReq(int id, char req, std::string message);
        
        void sendResponseWithMessage(const int id, const char reqId, const char response[]) const;
        void sendSimpleResponse(const int id, const char reqId) const;
        void startGame(int id0);
        void endGame(int id);
        void revealCell(int id, std::string message);
        void gameReconnect(int id, std::string message);
        int doReveal(int id, int i, int j);
        void surrenderGame(int id);

        void executeKick(int id);

        void executeCommand(std::string * command);
    
    public:

        Server(int bufferSize, int serverQueueSize, int maxClients);
        ~Server();

        int startToListen();
        int start(int port);

        void executeRevealResponse(int playerId, int i, int j, int mines, bool isEndGame);
        void executeEndGameResponse(int winner, int loser, const char reason);

        void sendMessage(const int id, const char req, const char message[]);

        void pushClientToIdleQueue(int id);
        void executeIdleQueue();

        void checkUserInput();   
    };

#endif