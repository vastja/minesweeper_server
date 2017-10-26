#ifndef _UTILS_HPP_
#define _UTILS_HPP_

    void arrayInitialize(int array[], int size);
    int findFreeId(int id[], int size);
    void freeId(int id[], int clientSocket, int size);
    int findId(int id[], int clientsSocket, int size);
    void convertIdToByteArray(int i, char bytes[]);

#endif