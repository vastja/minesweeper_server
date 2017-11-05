#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include "Game.hpp"

const char SEND_ID = 0;
const char START_GAME = 1;   

class Server {
    
    private:       
    
        std::queue<int> waitingForGame;
        Game *games;

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

        void sendMessage(int id, char req);
        void executeReq(int id, char req, char message[]);
        void sendId(int id);
        void sendStartGame(int id);
        void startGame(int id0);
    
    public:
        Server(int bufferSize, int serverQueueSize, int maxClients);
        ~Server();
        
    };

#endif