#include "Utils.hpp"
#include "Game.hpp"
#include "Server.hpp"
#include "Client.hpp"

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
void convert16bIdToByteArray(int i, char * fByte, char * sByte) {

    *fByte = i >> 8 & 0xFF;
    *sByte = i & 0xFF;

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

void getIds(std::string * recieved, int *id, char *reqId) {
    //TODO is valid format?
    *id = convertByteArrayTo16bId((*recieved)[0], (*recieved)[1]);
    *reqId = (*recieved)[2];

}

void parseMessage(int startPos, std::string * str, std::vector<std::string> * vec) {

    std::ostringstream os;

    bool escape = false;
    os.str("");
    for (int i = startPos; i < str->length(); i++) {

        if (escape) {
            escape = false;
            os << (*str)[i];
        }
        if ((*str)[i] == ESCAPE_CHAR) {
            escape = true;
        }
        else if ((*str)[i] == SEPARATOR_CHAR) {
            vec->push_back(os.str());
            os.str("");
        }
        else {
            os << (*str)[i];
        }
    }
}

void parseBuffer(char buffer[], int readyToRead, std::list<std::string> * reqList) {

    std::ostringstream os;

    int id;
    int reqId;

    bool escape = false;
    bool start = false;
    for (int i = 0; i < readyToRead; i++) {
        if (escape) {
            escape = false;
            if (start) {
                os << buffer[i];
            }     
        }
        else if (buffer[i] == ESCAPE_CHAR) {
            escape = true;
        }
        else if (buffer[i] == STX) {
            start = true;
            os.str("");
        }
        else if (buffer[i] == ETX) {
            start = false;
            reqList->push_back(os.str());
        } 
        else {
            if (start) {
                os << buffer[i];
            }
        }
    }
}

void generateGameCode(char *s, const int len) {

    srand(time(NULL));

    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len - 1; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len - 1] = '\0';
}