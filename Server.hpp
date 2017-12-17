#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <queue> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <semaphore.h>

const char SEND_ID = 4;
const char START_GAME = 5;
const char REVEAL = 6;
const char WIN = 7;
const char LOSE = 8;
const char DRAW = 9;
const char START_TURN = 10;
const char END_GAME_REVEAL = 11;
const char END_GAME = 12;
const char TIMEOUT_WIN = 13;
const char TIMEOUT_LOSE = 14;
const char SURRENDER_WIN = 15;
const char SURRENDER_LOSE = 16;
const char ALIVE = 17;
const char RECONECT = 18;
const char RECONECT_REFUSED = 19;
const char SERVER_STOP = 20;

const int CONNECTION_TIMEOUT = 10;

class Game; // forward decleration
class Client;

class Server {
    
    private:

        sem_t semaphore;
        std::queue<Client *> idleQueue;       
    
        std::queue<int> waitingForGame;
        Game **games;
        Client **clients;
        

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

        void sendMessage(int id, char req, char message[]);
        void executeReq(int id, char req, char message[]);
        
        void sendRevealed(int id, char reqId, char response[]);
        void sendSimpleResponse(int id, char reqId);
        void startGame(int id0);
        void endGame(int id);
        void revealCell(int id, char message[]);
        void gameReconnect(int id, char message[]);
        int doReveal(int id, int i, int j);

        void executeKick(Client * clients);
    
    public:

        Server(int bufferSize, int serverQueueSize, int maxClients);
        ~Server();

        int startToListen();
        int start();

        void executeRevealResponse(int playerId, int i, int j, int mines, bool isEndGame);
        void executeEndGameResponse(int winner, int loser, const char reason);
        
        void executeStartTurnResponse(int id) {sendMessage(id, START_TURN, NULL);};
        void executeStartGameResponse(int id, char gameCode[]) {sendMessage(id, START_GAME, gameCode);}
        void executeAlive(int id) {sendMessage(id, ALIVE, NULL);};

        void pushClientToIdleQueue(Client * client);
        void executeIdleQueue();
        
        
    };

#endif