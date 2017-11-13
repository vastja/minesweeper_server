#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <queue> 
#include <string>
#include <sstream>

#include "Server.hpp"
#include "Utils.hpp"
#include "Game.hpp"

int main() {

    Server server = Server(256, 16, 65536);
    server.~Server();

}

//I has to be sure there is no more clients than 16bit number
Server::Server(int bufferSize, int serverQueueSize, int maxClients) {

    this->bufferSize = bufferSize;
    this->maxClients = maxClients;
    cbuf = new char[bufferSize];
    clientsIds = new int[maxClients];
    *games = new Game[maxClients];
    arrayInitialize(clientsIds, maxClients);

    start();
    startToListen(serverQueueSize);
}

Server::~Server() {
    stop();
    delete[] cbuf;
    delete[] clientsIds;
    // TODO free all games
    delete[] games;
    cbuf = NULL;
    clientsIds = NULL;
    games = NULL;
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

    ioctl(fd, FIONREAD, &readyToRead );
               
    if (readyToRead > 0){
        recv(fd, cbuf, sizeof(char) * bufferSize, 0);
        //TODO
        int id;
        char reqId;
        getIds(cbuf, &id, &reqId);
        executeReq(id, reqId, cbuf);
              
    }
    else {
        close(fd);
        FD_CLR( fd, &client_socks );
        std::cout << "Klient se odpojil a byl odebran ze sady socketu\n";
        freeId(clientsIds, fd, maxClients);
    }
}

void Server::acceptNewClient(int fd) {

    if (FD_SETSIZE < maxClients) { 

        struct sockaddr_in peer_addr;
        socklen_t len_addr;

        int client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
        FD_SET( client_socket, &client_socks);
        std::cout << "Pripojen novy klient a pridan do sady socketu\n";

        int clientId = findFreeId(clientsIds, maxClients);
        std::cout << "Novy klient dostal id: "<< clientId << std::endl;

        clientsIds[clientId] = client_socket;
        sendMessage(clientId, SEND_ID, NULL);
        

    }
    else {
        // Send we are sorry but all seats are taken
    }
}

void Server::sendMessage(int id, char req, char message[]) { 
    
    switch (req) {
        case SEND_ID: 
        case START_GAME:
        case WIN:
        case LOSE:
        case DRAW:
        case START_TURN:
            sendSimpleResponse(id, req);
            break;
        case REVEAL:
            sendRevealed(id, message);
            break;
        default:
            std::cout << "This reqId code does not have set action\n";
    }
}

void Server::executeReq(int id, char req, char message[]) {

    switch (req) {
        case START_GAME:
            std::cout << "Executing START GAME request\n";
            startGame(id);
            break;
        case REVEAL:
            std::cout << "Executing REVEAL request\n";
            revealCell(id, message);
            break;
        default:
            std::cout << "Request with such ID does not exist\n";
    }
}

void Server::sendSimpleResponse(int id, char reqId) {
    char message[5];

    message[0] = STX;
    convert16bIdToByteArray(id, message + 1);
    message[3] = reqId;
    message[4] = ETX;
    
    int i = send(clientsIds[id], message, 5, 0);
}

void Server::sendRevealed(int id, char response[]) {

    int length = strlen(response) + 4;

    char message[length];
    message[0] = STX;
    convert16bIdToByteArray(id, message + 1);

    message[3] = REVEAL;

    strcpy(message + 4, response);
    printf ("response: %s\n", response);

    //char mes[10] = {'\000', '\000', '\002', '1', ';', '1', ';', '-', '1', ETX};
    int i = send(clientsIds[id], message, length, 0);
}

void Server::startGame(int id0) {

    if (!waitingForGame.empty()) {
        // TODO remember who is already waiting or enable start button ...
        int id1 = waitingForGame.front();
        waitingForGame.pop();
        Game *game = new Game(id0, id1); //Default width X height
        game->printBoard();
        games[id0] = game;
        games[id1] = game;
        sendMessage(id0, START_GAME, NULL);
        sendMessage(id1, START_GAME, NULL);
        sendMessage(id0, START_TURN, NULL);
    }
    else {
        waitingForGame.push(id0);
    }
}

void Server::revealCell(int id, char message[]) {

    char *parsedMessage = parseMessage(message + 3, bufferSize);
    const char *delim = ";";
    char *token;

    token = strtok(parsedMessage, delim);
    int i = atoi(token);
    token = strtok(NULL, delim);
    int j = atoi(token);


    doReveal(id, i,j);

    int oponentId = games[id]->getOponent(id);
    sendMessage(oponentId, START_TURN, NULL);

    delete parsedMessage;
    parsedMessage = NULL;
}

void Server::doReveal(int id, int i, int j) {

    if (games[id]->isRevealed(i, j)) {
        return;
    }

    int oponentId = games[id]->getOponent(id);
    int mines = games[id]->reveal(i, j);

    std::ostringstream oss;
    oss << i << ';' << j << ';';
    if (mines == MINE) {

        oss << MINE;
        std::string str = oss.str() + ETX;
        std::cout << "1: " << str << std::endl;
        char message[str.length()];
        strcpy(message, str.c_str());

        sendMessage(id, LOSE, message);
        sendMessage(oponentId, WIN, message);
    }
    else if (mines == 0) {

        oss << mines;
        std::string str = oss.str() + ETX;
        char message[str.length()];
        strcpy(message, str.c_str());
        sendMessage(id, REVEAL, message);
        sendMessage(oponentId, REVEAL, message);

        for (int w = i - 1; w <= i + 1; w++) {
            for (int h = j - 1; h <= j + 1; h++) {
                if (checkRange(w, h, games[id]->width, games[id]->height)) {
                    doReveal(id, w, h);
                }
            }
        }

    }
    else {

        oss << mines;
        std::string str = oss.str() + ETX;
        char message[str.length()];
        strcpy(message, str.c_str());

        sendMessage(id, REVEAL, message);
        sendMessage(oponentId, REVEAL, message);
    }

    if (!games[id]->areFieldsToPlay()) {
        sendMessage(id, DRAW, NULL);
        sendMessage(oponentId, DRAW, NULL);
    }
}
