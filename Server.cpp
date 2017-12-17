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
#include "Game.hpp"
#include "Client.hpp"
#include "Utils.hpp"

int main() {

    Server server = Server(256, 16, 65536);
    server.start();
    server.startToListen();
    server.~Server();

}

//I have to be sure there is no more clients than 16bit number
Server::Server(int bufferSize, int serverQueueSize, int maxClients) {

    sem_init(&semaphore, 0, 1);

    // TODO HOW BIG IS FD_SET???
    this->bufferSize = bufferSize;
    this->maxClients = maxClients;
    this->serverQueueSize = serverQueueSize;

    cbuf = new char[bufferSize];
    clients = new Client * [maxClients] {NULL};
    games = new Game * [maxClients] {NULL};
}

Server::~Server() {
    stop();

    for (int i = 0; i < maxClients; i++) {
        if (games[i] != NULL) {
            int a, b;
            games[i]->getPlayers(&a, &b);
            delete games[i];
            games[a] == NULL;
            games[b] == NULL;
        }
    }

    delete [] cbuf;

    for (int i; i < maxClients; i++) {
        if (clients[i] != NULL) {
            delete clients[i];
            clients[i] = NULL;
        }
    }

    delete [] clients;
    delete [] games;
   

    cbuf = NULL;
    clients = NULL;
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
    close(server_socket);
}

int Server::startToListen() {

    int return_value = listen(server_socket, serverQueueSize);
    if (return_value == 0){
        std::cout << "Listen - OK\n";
    } else {
        std::cout << "Listen - ER\n";
        return -1;
    }

    FD_ZERO(&client_socks);
	FD_SET(server_socket, &client_socks);

	for (;;){

        timeval timeout;
        timeout.tv_sec = Client::NORMAL_TIME;

		fd_set clientsSet = client_socks;
		return_value = select( FD_SETSIZE, &clientsSet, ( fd_set *)0, ( fd_set *)0, &timeout);

        executeIdleQueue();

		if (return_value < 0) {
			std::cout << "Select - ERR\n";
			return -1;
        }
        
		// vynechavame stdin, stdout, stderr
		for(int fd = 3; fd < FD_SETSIZE; fd++ ){
            execute(clientsSet, fd);
        }
    }

}

void Server::execute(fd_set clientsSet, int fd) {

    if( FD_ISSET( fd, &clientsSet ) ){
       
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

           
    if (readyToRead > 0) {
        // Shortest valid message is 5 chars long
        if (readyToRead >= 5) {
            recv(fd, cbuf, sizeof(char) * bufferSize, 0);
            int id;
            char reqId;
            getIds(cbuf, &id, &reqId);
            executeReq(id, reqId, cbuf);
        }
        else {
            return;
        }
    }
    else {
        int id = findClientId(clients, fd, maxClients);
        executeKick(clients[id]);
    }
}

void Server::acceptNewClient(int fd) {

    // In case there is space for new client
    if (FD_SETSIZE < maxClients) { 

        struct sockaddr_in peer_addr;
        socklen_t len_addr;

        int clientId = findFreeId(clients, maxClients);

        if (clientId == -1) {
            std::cout << "Server run out of free ids\n";
            return;
        }

        int client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
        FD_SET( client_socket, &client_socks);
        std::cout << "Pripojen novy klient with socket: " << client_socket << " pridan do sady socketu\n";

        std::cout << "Novy klient dostal id: "<< clientId << std::endl;

        clients[clientId] = new Client(clientId, client_socket, this);
        clients[clientId]->startTimer(Client::NORMAL_TIME);
        
        sendMessage(clientId, SEND_ID, NULL);
    }
}

void Server::sendMessage(int id, char req, char message[]) { 
    
    // In case client is disconnected do not send
    if (clients[id] == NULL) {
        return;
    }

    switch (req) {
        case SEND_ID: 
        case START_GAME:
        case WIN:
        case LOSE:
        case DRAW:
        case START_TURN:
        case END_GAME:
        case ALIVE:
        case TIMEOUT_LOSE:
        case TIMEOUT_WIN:
        case RECONECT_REFUSED:
            sendSimpleResponse(id, req);
            break;
        case END_GAME_REVEAL:
        case REVEAL:
            sendRevealed(id, req, message);
            break;
        default:
            std::cout << "This reqId code does not have set action\n";
    }
}

void Server::executeReq(int id, char req, char message[]) {

    // In case there is not such a client
    if (clients[id] == NULL) {
        return;
    }

    switch (req) {
        case START_GAME:
            std::cout << "Executing START GAME request\n";
            startGame(id);
            break;
        case REVEAL:
            std::cout << "Executing REVEAL request\n";
            revealCell(id, message);
            break;
        case END_GAME:
            std::cout << "Executing END GAME request\n";
            endGame(id);
            break;
        case ALIVE:
            std::cout << "Executing ALIVE request\n";
            break;
        default:
            std::cout << "Request with such ID does not exist\n";
    }

    time_t now = time(0);
    clients[id]->setLastCommunication(now);
}

void Server::sendSimpleResponse(int id, char reqId) {
    char message[5];

    message[0] = STX;
    convert16bIdToByteArray(id, message + 1);
    message[3] = reqId;
    message[4] = ETX;
    
    int i = send(clients[id]->getClientsSocket(), message, 5, 0);
}

void Server::sendRevealed(int id, char reqId, char response[]) {

    int length = strlen(response) + 4;

    char message[length];
    message[0] = STX;
    convert16bIdToByteArray(id, message + 1);

    message[3] = reqId;

    strcpy(message + 4, response);
    printf ("response: %s\n", response);

    int i = send(clients[id]->getClientsSocket(), message, length, 0);
}

void Server::startGame(int id0) {

    // Is player in game or is already waiting for game?
    if (isInGame(games, maxClients, id0) || clients[id0]->isWaitingForGame()) {
        return;
    }

    if (!waitingForGame.empty()) {

        int id1 = waitingForGame.front();
        waitingForGame.pop();
        Game *game = new Game(this, id0, id1); //Default width X height
        game->printBoard();
        
        games[id0] = game;
        games[id1] = game;

        clients[id0]->setWaitingForGame(false);
        clients[id0]->setWaitingForGame(false);
    }
    else {
        waitingForGame.push(id0);
        clients[id0]->setWaitingForGame(true);
    }
}

void Server::endGame(int id) {

    if (!isInGame(games, maxClients, id)) {
        return;
    }

    games[id]->endGame(id);
}

void Server::revealCell(int id, char message[]) {

    if (games[id]->getTurn() != id || !isInGame(games, maxClients, id)) {
        return;
    }

    char result[bufferSize];
    parseMessage(message + 4, bufferSize, result);
    const char *delim = ";";
    char *token;

    try {
        token = strtok(result, delim);
        if (token == NULL) {
            return;
        }
        int i = std::stoi(token);

        token = strtok(NULL, delim);
         if (token == NULL) {
            return;
        }
        int j = std::stoi(token);
        doReveal(id, i,j);
    }
    catch (std::invalid_argument) {
        return;
    }
    catch (std::out_of_range) {
        return;
    }
}

int Server::doReveal(int id, int i, int j) {
    games[id]->doReveal(i, j);
}

void Server::executeRevealResponse(int playerId, int i, int j, int mines, bool isEndGame) {
    
    std::ostringstream oss;
    std::string str;

    oss << i << ';' << j << ';' << mines;
    str = oss.str() + ETX;

    char message[str.length()];
    strcpy(message, str.c_str());

    if (isEndGame) {
        sendMessage(playerId, END_GAME_REVEAL, message);
    }
    else {
        sendMessage(playerId, REVEAL, message);
    }
}

void Server::executeEndGameResponse(int winner, int loser, const char reason) {

    switch (reason) {
        case Game::DEATH:
            sendMessage(winner, WIN, NULL);
            sendMessage(loser, LOSE, NULL);
            games[winner]->endGameReveal();
            break;
        case Game::DRAW:
            sendMessage(winner, DRAW, NULL);
            sendMessage(loser, DRAW, NULL);
            break;
        case Game::TIMEOUT:
            sendMessage(winner, TIMEOUT_WIN, NULL);
            sendMessage(loser, TIMEOUT_LOSE, NULL);
            games[winner]->endGameReveal();
            break;
        case Game::SURRENDER:
            sendMessage(winner, SURRENDER_WIN, NULL);
            sendMessage(loser, SURRENDER_LOSE, NULL);
            games[winner]->endGameReveal();
            break;
    }

    delete games[winner];
    games[winner] = NULL;
    games[loser] = NULL;
}

void Server::executeKick(Client * client) {

    client->cancelTimer();

    int fd =  client->getClientsSocket();
    
    int i = close(fd);
    FD_CLR(fd, &client_socks);

    std::cout << "Klient with socket: " << fd << " se odpojil a byl odebran ze sady socketu\n"; 

    delete client;
    client = NULL;
}

void Server::gameReconnect(int id, char message[]) {

    char * result = new char[bufferSize];
    parseMessage(message + 4, bufferSize, result);
    const char *delim = ";";
    char *token;

    try {
        token = strtok(result, delim);
        if (token == NULL) {
            return; 
        }

        int oldId = std::stoi(token);
        token = strtok(NULL, delim);
        if (token == NULL) {
            return; 
        }

        if (games[oldId] != NULL && games[oldId]->reconnect(id, token)) {
            sendMessage(id, RECONECT, NULL);
            games[id]->revealStateOfGame(id);
        }
        else {
            sendMessage(id, RECONECT_REFUSED, NULL);
        }
    }
    catch (std::invalid_argument) {
        return;
    }
    catch (std::out_of_range) {
        return;
    }
}

void Server::pushClientToIdleQueue(Client * client) {
    sem_wait(&semaphore);
    idleQueue.push(client);
    sem_post(&semaphore);
}

void Server::executeIdleQueue() {
    sem_wait(&semaphore);
    while(!idleQueue.empty()) {
        Client * client = idleQueue.front();
        executeKick(client);
        idleQueue.pop();
    }
    sem_post(&semaphore);
}
