#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <sstream>
#include <list>

#include "Server.hpp"

    class Game;

    const char STX = '\x02';
    const char ETX = '\x03';
    const char ESCAPE_CHAR = '/';
    const char SEPARATOR_CHAR = ';';

    void arrayInitialize(Client clients[], int size);

    void gamesInitialize(Game ** array, int size);

    int findFreeId(Client * clients[], int size);

    void freeClient(Client * clients[], int clientSocket, int size);

    int findClientId(Client * clients[], int clientsSocket, int size);

    void convert16bIdToByteArray(int i, char  * sByte, char * fByte);

    int convertByteArrayTo16bId(char byte0, char byte1);

    bool checkRange(int i, int j, int width, int height);

    void getIds(std::string * recieved, int *id, char *reqId);

    void parseMessage(char message[], int bufferSize, char result[]);

    bool isInGame(Game ** games, int size, int id);

    void generateGameCode(char *s, const int len);

    void parseBuffer(char buffer[], int readyToRead, std::list<std::string> * reqList);
    void parseMessage(int startPos, std::string * str, std::vector<std::string> * vec); 

#endif