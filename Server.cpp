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
#include "IdleChecker.hpp"

int main(int argc, char * argv[]) {

    if (argc == 2) {
        int port = atoi(argv[1]);

        Server server = Server(1024, 256, 2048);
        int state = server.start(port);


        if (state != -1) {
            std::thread inputThread (&Server::checkUserInput, &server);
            inputThread.detach();

            server.startToListen();
        }
    }
    else {
        std::cout << "USAGE Server.out <port>\n";
    }

    exit(0);
}

void Server::checkUserInput() {
    
    std::string command;
    
    while(run) {
        std::cout << ">> ";
        std::cin >> command;

        if(command == "EXIT" || command == "exit") {
            run = false;
            break;
        }
        else {
            executeCommand(&command);
        }   
    }
}

void Server::executeCommand(std::string * command) {
    
    std::string arg;

    if((*command) == "KICK" || (*command) == "kick") {
        std::cin >> arg;
        int i = stoi(arg);
        pushClientToIdleQueue(i);
    }
    else if ((*command) == "STATS" || (*command) == "stats") {
        std::cout << "SERVER STATISTICS\n";
        std::cout << "Clients connected: " << info.clientsConnected << std::endl;
        std::cout << "Clients disconnected: " << info.clientsDisconnected << std::endl;
        std::cout << "Clients idle disconnected: " << info.clientsIdleDisconnected << std::endl;
        std::cout << "Sent messages " << info.sentCount << std::endl;
        std::cout << "Received messages: " << info.recievedCount << std::endl;
        std::cout << "Invalid messages received: " << info.notValidMessagesCount << std::endl;
        std::cout << "Invalid requests: " << info.notValidRequestsCount << std::endl;
    }
    else if ((*command) == "GAME-BOARD" || (*command) == "game-board") {
        std::cin >> arg;
        int i = stoi(arg);

        if (i >= 0 && i <= maxClients) {
            if (games[i] != NULL) {
                games[i]->printBoard();
            }
        }
    }
}

//I have to be sure there is no more clients than 16bit number
Server::Server(int bufferSize, int serverQueueSize, int maxClients) {

    sem_init(&semaphore, 0, 1);

    this->bufferSize = bufferSize;

    if (maxClients > MAX_CLIENTS_LIMIT) {
        this->maxClients = MAX_CLIENTS_LIMIT;
    }

    this->maxClients = maxClients;
    this->serverQueueSize = serverQueueSize;

    cbuf = new char[bufferSize];
    clients = new Client * [maxClients] {NULL};
    games = new Game * [maxClients] {NULL};

    idleChecker = new IdleChecker(this, clients, maxClients);
    idleChecker->start();
}

Server::~Server() {

    run = false;
    idleChecker->cancel();
    close(server_socket);

    for (int i = 0; i < maxClients; i++) {
        if (games[i] != NULL) {
            int a, b;
            games[i]->getPlayers(&a, &b);
            delete games[i];
            games[a] = NULL;
            games[b] = NULL;
        }
    }

    delete [] cbuf;

    for (int i = 0; i < maxClients; i++) {
        if (clients[i] != NULL) {
            delete clients[i];
            clients[i] = NULL;
        }
    }

    delete [] clients;
    delete [] games;
    delete idleChecker;

    cbuf = NULL;
    clients = NULL;
    games = NULL;
    idleChecker = NULL;
}

int Server::start(int port) {

    run = true;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))) {
        std::cout << "Setsockopt failed\n";
        return -1;
    }
    
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    int return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));
    
    if (return_value == 0) 
        std::cout << "Bind - OK\n";
    else {
        std::cout << "Bind - ERR\n";
        return -1;
    }
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

	while(run){

        timeval timeout;
        timeout.tv_sec = IdleChecker::TIMEOUT;

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

            std::list<std::string> reqList;

            if (readyToRead > bufferSize) {
                readyToRead = bufferSize;
            }

            parseBuffer(cbuf, readyToRead, &reqList);

            for (std::list<std::string>::iterator it = reqList.begin(); it != reqList.end(); ++it) {
                
                if ((*it).length() < 3) {
                    info.notValidMessagesCount++;
                    int id = findClientId(clients, fd, maxClients);

                    if (id != -1) {
                        clients[id]->incBadFormatMessagesCount();
                        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
                    }

                    continue;
                }

                int id;
                char reqId;
                getIds(&(*it), &id, &reqId);

                // INDEX out of borders - shouldnt be possible
                if (id < 0 || id > (maxClients - 1)) {
                    continue;
                }

                //char message[(*it).length()];
                //memcpy(&message, (*it).c_str(), (*it).length());
                std::string message = *it;
                executeReq(id, reqId, message);
            }
            
        }
        else {
            recv(fd, cbuf, sizeof(char) * bufferSize, 0);

            info.notValidMessagesCount++;
            int id = findClientId(clients, fd, maxClients);

            if (id != -1) {
                clients[id]->incBadFormatMessagesCount();
                sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
            }

            info.notValidMessagesCount++;
            // std::cout << "Not valid message recieved\n";
            return;
        }
    }
    else {
        //STAT-INFO
        info.clientsDisconnected++;
        int id = findClientId(clients, fd, maxClients);
        executeKick(id);
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

        clients[clientId] = new Client(clientId, client_socket);
        
        //STAT-INFO
        info.clientsConnected++;

        sendMessage(clientId, SEND_ID, NULL);
    }
}

void Server::sendMessage(const int id, const char req, const char message[]) { 
    
    // In case client is disconnected do not send
    if (clients[id] == NULL) {
        return;
    }

    if (message == NULL) {
        std::cout << "SEND reqId: " << (int) req << " to client with ID: " << id << " with no message\n";
    }
    else {
        std::cout << "SEND reqId: " << (int) req << " to client with ID: " << id << " with message: " << message << std::endl;
    }
    

    switch (req) { 
        case WIN:
        case LOSE:
        case DRAW:
        case END_TURN:
        case START_TURN:
        case TIMEOUT_LOSE:
        case TIMEOUT_WIN:
        case SURRENDER_LOSE:
        case SURRENDER_WIN:
        case END_GAME:
        case SEND_ID:
        case ALIVE:
        case START_GAME_ACCEPTED:
        case RECONNECT:
            sendSimpleResponse(id, req);
            break;
        case END_GAME_REVEAL:
        case REVEAL:
        case START_GAME:
        case RECONNECT_REFUSED:
        case START_GAME_REFUSED:
        case REVEAL_REFUSED:
        case END_GAME_REFUSED:
        case SURRENDER_REFUSED:
        case MESSAGE_BAD_FORMAT:
            sendResponseWithMessage(id, req, message);
            break;
        default:
            std::cout << "This reqId code does not have set action\n";
    }

    //STAT-INFO
    info.sentCount++;
}

void Server::executeReq(int id, char req, std::string message) {

    //STAT-INFO
    info.recievedCount++;
    // In case there is not such a client
    if (clients[id] == NULL) {
        return;
    }

    switch (req) {
        case START_GAME:
            std::cout << "Executing START GAME request [" << id << "]\n";
            clients[id]->resetBadFormatMessagesCount();
            startGame(id);
            break;
        case REVEAL:
            std::cout << "Executing REVEAL request [" << id << "]\n";
            clients[id]->resetBadFormatMessagesCount();
            revealCell(id, message);
            break;
        case END_GAME:
            std::cout << "Executing END GAME request [" << id << "]\n";
            clients[id]->resetBadFormatMessagesCount();
            endGame(id);
            break;
        case ALIVE:
            std::cout << "Executing ALIVE request [" << id << "]\n";
            clients[id]->resetBadFormatMessagesCount();
            sendMessage(id, ALIVE, NULL);
            break;
        case RECONNECT:
            std::cout << "Executing RECONNECT request [" << id << "]\n";
            clients[id]->resetBadFormatMessagesCount();
            gameReconnect(id, message);
            break;
        case SURRENDER:
            std::cout << "Executing SURRENDER request [" << id << "]\n";
            clients[id]->resetBadFormatMessagesCount();
            surrenderGame(id);
            break;
        default:
            info.notValidRequestsCount++;
            clients[id]->incBadFormatMessagesCount();
            sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
            std::cout << "Request with such ID does not exist\n";
    }

    clients[id]->setLastCommunication();
}

void Server::sendSimpleResponse(const int id, const char reqId) const {
    
    char fByte;
    char sByte;

    convert16bIdToByteArray(id, &fByte, &sByte);

    std::ostringstream oss;

    oss << STX;

    if (fByte == ETX || fByte == STX) {
			oss << ESCAPE_CHAR;
	}
	oss << fByte;
		
	if (sByte == ETX || sByte == STX) {
		oss << ESCAPE_CHAR;
	}
	oss << sByte;
		
	oss << reqId;	
	oss << ETX;
    
    std::string message = oss.str();
    
    int i = send(clients[id]->getClientsSocket(), message.c_str(), message.length(), 0);
}

void Server::sendResponseWithMessage(const int id, const char reqId, const char response[]) const {

    char fByte;
    char sByte;

    convert16bIdToByteArray(id, &fByte, &sByte);

    std::ostringstream oss;

    oss << STX;

    if (fByte == ETX || fByte == STX) {
			oss << ESCAPE_CHAR;
	}
	oss << fByte;
		
	if (sByte == ETX || sByte == STX) {
		oss << ESCAPE_CHAR;
	}
	oss << sByte;
		
	oss << reqId;
    oss << std::string(response);	
    oss << SEPARATOR_CHAR;
	oss << ETX;
    
    std::string message = oss.str();

    int i = send(clients[id]->getClientsSocket(), message.c_str(), message.length(), 0);
}

void Server::startGame(int id0) {

    // Is player in game or is already waiting for game?
    if (clients[id0] == NULL) { 
        return;
    } 
    else if (clients[id0]->isInGame()) {
        sendMessage(id0, START_GAME_REFUSED, START_GAME_E1);
        return;
    }
    else if (clients[id0]->isWaitingForGame()) {
        sendMessage(id0, START_GAME_REFUSED, START_GAME_E2);
        return;
    }

    if (!waitingForGame.empty()) {

        int id1 = waitingForGame.front();
        waitingForGame.pop();

        if (clients[id1] == NULL) {
            startGame(id0);
        }
        else if (id0 == id1 || !clients[id1]->isWaitingForGame() || clients[id1]->isInGame()) {
            startGame(id0);
        }
        else {
            sendMessage(id0, START_GAME_ACCEPTED, NULL);
            Game *game = new Game(this, id0, id1); //Default width X height
            // game->printBoard();
            
            games[id0] = game;
            games[id1] = game;

            clients[id0]->setWaitingForGame(false);
            clients[id1]->setWaitingForGame(false);

            clients[id0]->setInGame(true);
            clients[id1]->setInGame(true);

            game->setTurn(id0);
            //STAT-INFO
            info.gamesStarted++;
        }
    }
    else {
        waitingForGame.push(id0);
        clients[id0]->setWaitingForGame(true);
        sendMessage(id0, START_GAME_ACCEPTED, NULL);
    }
}

void Server::endGame(int id) {

    if (clients[id] == NULL) {
        return;
    }
    else if (!clients[id]->isInGame()) {
        sendMessage(id, END_GAME_REFUSED, END_GAME_E2);
        return;
    }
    else if (games[id] == NULL) {
        sendMessage(id, END_GAME_REFUSED, END_GAME_E1);
        return;
    }

    sendMessage(id, END_GAME, NULL);

    clients[id]->setLastCommunication();
    clients[id]->setInGame(false);

    games[id]->setAway(id);
    games[id]->endGame(id);
}

void Server::surrenderGame(int id) {

    if (clients[id] == NULL) {
        return;
    }
    else if (!clients[id]->isInGame()) {
        sendMessage(id, SURRENDER_REFUSED, SURRENDER_E2);
        return;
    }
    else if (games[id] == NULL) {
        sendMessage(id, SURRENDER_REFUSED, SURRENDER_E1);
        return;
    }

    clients[id]->setLastCommunication();

    games[id]->endGameSurrender(id);
}

void Server::revealCell(int id, std::string message) {

    if (games[id] == NULL) {
        sendMessage(id, REVEAL_REFUSED, REVEAL_E2);
        return;
    }
    else if (games[id]->getTurn() != id) {
        sendMessage(id, REVEAL_REFUSED, REVEAL_E1);
        return;
    } 

    std::vector<std::string> vec; 

    parseMessage(3, &message, &vec);

    //NOT enough params
    if (vec.size() < 2) {
        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
        clients[id]->incBadFormatMessagesCount();
        return;
    }

    try {
       
        int i = std::stoi(vec[0]);
        int j = std::stoi(vec[1]);
        doReveal(id, i,j);
    }
    catch (std::invalid_argument) {
        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
        clients[id]->incBadFormatMessagesCount();
        return;
    }
    catch (std::out_of_range) {
        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
        clients[id]->incBadFormatMessagesCount();
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
    str = oss.str();

    char message[str.length() + 1];
    strcpy(message, str.c_str());
    message[str.length()] = '\0';

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
            games[winner]->endGameReveal(winner);
            games[loser]->endGameReveal(loser);
            break;
        case Game::DRAW:
            sendMessage(winner, DRAW, NULL);
            sendMessage(loser, DRAW, NULL);
            break;
        case Game::TIMEOUT:
            sendMessage(winner, TIMEOUT_WIN, NULL);
            sendMessage(loser, TIMEOUT_LOSE, NULL);
            games[winner]->endGameReveal(winner);
            games[loser]->endGameReveal(loser);
            break;
        case Game::SURRENDER:
            sendMessage(winner, SURRENDER_WIN, NULL);
            sendMessage(loser, SURRENDER_LOSE, NULL);
            games[winner]->endGameReveal(winner);
            games[loser]->endGameReveal(loser);
            break;
        case Game::EXIT:
            sendMessage(winner, SURRENDER_WIN, NULL);
            games[winner]->endGameReveal(winner);
            break;
    }

    if (clients[winner] != NULL) {
        clients[winner]->setInGame(false);
    }
    if (clients[loser] != NULL) {
        clients[loser]->setInGame(false);
    }

    delete games[winner];
    games[winner] = NULL;
    games[loser] = NULL;
}

void Server::executeKick(int id) {

    if (clients[id] == NULL) {
        return;
    }

    int fd =  clients[id]->getClientsSocket();
    
    int i = close(fd);
    FD_CLR(fd, &client_socks);

    if (clients[id]->isInGame()) {
        games[id]->setAway(id);
    }

    std::cout << "Klient with socket: " << fd << " byl odpojen a byl odebran ze sady socketu\n"; 

    delete clients[id];
    clients[id] = NULL;
}

void Server::gameReconnect(int id, std::string message) {

    std::vector<std::string> vec; 

    parseMessage(3, &message, &vec);

    //NOT enough params
    if (vec.size() < 2) {
        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
        clients[id]->incBadFormatMessagesCount();
        return;
    };

    try {

        int oldId = std::stoi(vec[0]);

        if (games[oldId] == NULL) {
            sendMessage(id, RECONNECT_REFUSED, RECONNECT_E2);
            return;
        }
        else if (games[oldId]->reconnect(id, oldId, vec[1].c_str())) {
            clients[id]->setInGame(true);
            games[id]->revealStateOfGame(id);
        }
    }
    catch (std::invalid_argument) {
        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
        clients[id]->incBadFormatMessagesCount();
        return;
    }
    catch (std::out_of_range) {
        sendMessage(id, MESSAGE_BAD_FORMAT, BAD_FORMAT_ERROR_E1);
        clients[id]->incBadFormatMessagesCount();
        return;
    }
}

void Server::pushClientToIdleQueue(int id) {
    sem_wait(&semaphore);
    idleQueue.push(id);
    sem_post(&semaphore);
}

void Server::executeIdleQueue() {
    sem_wait(&semaphore);
    while(!idleQueue.empty()) {
        int id = idleQueue.front();
        info.clientsIdleDisconnected++;
        executeKick(id);
        idleQueue.pop();
    }
    sem_post(&semaphore);
}
