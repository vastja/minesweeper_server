#include "Utils.hpp"

void arrayInitialize(int array[], int size) {
    for (int i = 0; i < size; i++) {
        array[i] = -1;
    }
}

int findFreeId(int id[], int size) {

    for (int i = 0; i < size; i++) {
        if (id[i] == -1) {
            return i;
        }
    }

    return -1;
}

void freeId(int id[], int clientSocket, int size) {
    for (int i = 0; i < size; i++) {
        if (id[i] == clientSocket) {
            id[i] = -1;
        }
    }
}

int findId(int id[], int clientSocket, int size) {

    for (int i = 0; i < size; i++) {
        if (id[i] == clientSocket) {
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

    *id = convertByteArrayTo16bId(recieved[0], recieved[1]);
    *reqId = recieved[2];

}