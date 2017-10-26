#ifndef _SERVER_HPP_
#define _SERVER_HPP_

class Server {
    
    private:
    
        // For given id save socket
        int *id;
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
    
    public:
        Server(int bufferSize, int serverQueueSize, int maxClients);
        ~Server();
        
    };

#endif