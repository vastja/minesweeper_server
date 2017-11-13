#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include "Game.hpp"

const char SEND_ID = 4;
const char START_GAME = 5;
const char REVEAL = 6;
const char WIN = 7;
const char LOSE = 8;
const char DRAW = 9;
const char START_TURN = 10;

class Server {
    
    private:       
    
        std::queue<int> waitingForGame;
        Game **games;

        // For given id save socket
        int *clientsIds;
        int maxClients;
    
        int server_socket;
        struct sockaddr_in my_addr;
        
        char *cbuf;
        int bufferSize;
    
        fd_set client_socks;
        
        int startToListen(int serverQueueSize);
        int start();
        int stop();
        void execute(fd_set clients, int fd);
        void read(int fd);
        void acceptNewClient(int fd);

        void sendMessage(int id, char req, char message[]);
        void executeReq(int id, char req, char message[]);
        
        void sendRevealed(int id, char response[]);
        void sendSimpleResponse(int id, char reqId);
        void startGame(int id0);
        void revealCell(int id, char message[]);
        void doReveal(int id, int i, int j);
    
    public:
        Server(int bufferSize, int serverQueueSize, int maxClients);
        ~Server();
        
    };

#endif