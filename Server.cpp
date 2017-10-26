#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "Server.hpp"
#include "Utils.hpp"

int main() {

    Server server = Server(256, 16, 65536);
    server.~Server();

}

//I has to be sure there is no more clients than 16bit number
Server::Server(int bufferSize, int serverQueueSize, int maxClients) {

    this->maxClients = maxClients;
    cbuf = new char[bufferSize];
    id = new int[maxClients];
    arrayInitialize(id, maxClients);

    start();
    startToListen(serverQueueSize);
}

Server::~Server() {
    stop();
    delete cbuf;
    delete id;
    cbuf = NULL;
    id = NULL;
}

int Server::start() {

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(60000);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    int return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));
    
    if (return_value == 0) 
        std::cout << "Bind - OK\n";
    else {
        std::cout << "Bind - ERR\n";
        return -1;
    }
}

int Server::stop() {
    for (int fd = 3; fd < FD_SETSIZE; fd++) {
        // TODO send message that server stoped
        close(fd);
    }
    close(server_socket);
}

int Server::startToListen(int serverQueueSize) {

    int return_value = listen(server_socket, serverQueueSize);
    if (return_value == 0){
        std::cout << "Listen - OK\n";
    } else {
        std::cout << "Listen - ER\n";
        return -1;
    }

    FD_ZERO(&client_socks );
	FD_SET(server_socket, &client_socks );

	for (;;){

		fd_set clients = client_socks;
		return_value = select( FD_SETSIZE, &clients, ( fd_set *)0, ( fd_set *)0, ( struct timeval *)0 );

		if (return_value < 0) {
			std::cout << "Select - ERR\n";
			return -1;
        }
        
		// vynechavame stdin, stdout, stderr
		for(int fd = 3; fd < FD_SETSIZE; fd++ ){
            execute(clients, fd);
        }
    }

}

void Server::execute(fd_set clients, int fd) {

    if( FD_ISSET( fd, &clients ) ){
       
        if (fd == server_socket){
            acceptNewClient(fd);      
        }
        // je to klientsky socket ? prijmem data
        else {
            read(fd);
        }
    }
}

void Server::read(int fd) {
    
    int readyToRead;

    ioctl( fd, FIONREAD, &readyToRead );
               
    if (readyToRead > 0){
        recv(fd, cbuf, sizeof(char) * 256, 0);
        std::cout << "Prijato: " << cbuf << std::endl;
              
    }
    else {
        close(fd);
        FD_CLR( fd, &client_socks );
        std::cout << "Klient se odpojil a byl odebran ze sady socketu\n";
        freeId(id, fd, maxClients);
    }
}

// Paralelne??
void Server::acceptNewClient(int fd) {

    if (FD_SETSIZE < maxClients) { 

        struct sockaddr_in peer_addr;
        socklen_t len_addr;

        int client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
        FD_SET( client_socket, &client_socks);
        std::cout << "Pripojen novy klient a pridan do sady socketu\n";

        int clientId = findFreeId(id, maxClients);
        std::cout << "Novy klient dostal id: "<< clientId << std::endl;

        char message[4];

        convertIdToByteArray(clientId, message);

        message[2] = 0;
        message[3] = 3;
        std::cout << "Here 1\n";
        
        int i = send(client_socket, message, 4, 0);
        std::cout << "Here " << i << std::endl;
        // TODO overit, ze dostal id
        id[clientId] = client_socket;

    }
    else {
        // Send we are sorry but all seats are taken
    }


}
