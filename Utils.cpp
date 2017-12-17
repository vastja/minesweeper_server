#include "Utils.hpp"
#include "Game.hpp"
#include "Server.hpp"
#include "Client.hpp"

const char ESCAPE_CHAR = '/';

int findFreeId(Client ** clients, int size) {

    for (int i = 0; i < size; i++) {
        if (clients[i] == NULL) {
            return i;
        }
    }

    return -1;
}

void freeClient(Client ** clients, int clientSocket, int size) {
    for (int i = 0; i < size; i++) {
        if (clients[i]->getClientsSocket() == clientSocket) {
            delete clients[i];
            clients[i] = NULL;
        }
    }
}

int findClientId(Client ** clients, int clientSocket, int size) {

    for (int i = 0; i < size; i++) {
        if (clients[i] != NULL && clients[i]->getClientsSocket() == clientSocket) {
            return i;
        }
    }

    return -1;
}

// 16 bits - 
void convert16bIdToByteArray(int i, char bytes[]) {

    bytes[0] = i >> 8 & 0xFF;
    bytes[1] = i & 0xFF;

}

int convertByteArrayTo16bId(char byte0, char byte1) {

    int i = byte0 << 8;
    i = i | byte1;
    return i;

}

//TODO inline
bool checkRange(int i, int j, int width, int height) {
    return i >= 0 && i < width && j >= 0 && j < height;
}

void getIds(char recieved[], int *id, char *reqId) {
    //TODO is valid format?
    *id = convertByteArrayTo16bId(recieved[1], recieved[2]);
    *reqId = recieved[3];

}

void parseMessage(char message[], int bufferSize, char result[]) {

    // TODO add buffer as parameter 
    int pos = 0;

    bool escape = false;
    for (int i = 0; i < bufferSize; i++) {

        if (message[i] == ESCAPE_CHAR) {
            escape = true;
        }
        else if (message[i] == ETX && escape == false) {
            result[pos] = '\0';
            break;
        }
        else {
            result[pos++] = message[i];
            escape = false;
        }

    }
}

bool isInGame(Game ** games, int size, int id) {

    if (id >= 0 && id < size) {
        return games[id] != NULL; 
    }
    else {
        return false;
    }
}

void generateGameCode(char *s, const int len) {

    srand(time(NULL));

    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = '\0';
}