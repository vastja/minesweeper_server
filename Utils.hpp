#ifndef _UTILS_HPP_
#define _UTILS_HPP_

    const char STX = '\x02';
    const char ETX = '\x03';

    void arrayInitialize(int array[], int size);
    int findFreeId(int id[], int size);
    void freeId(int id[], int clientSocket, int size);
    int findId(int id[], int clientsSocket, int size);
    void convert16bIdToByteArray(int i, char bytes[]);
    int convertByteArrayTo16bId(char byte0, char byte1);
    bool checkRange(int i, int j, int width, int height);
    void getIds(char recieved[], int *id, char *reqId);
    char *parseMessage(char message[], int bufferSize);

#endif