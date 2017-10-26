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
void convertIdToByteArray(int i, char bytes[]) {

    bytes[0] = i >> 8 & 0xFF;
    bytes[1] = i & 0xFF;

}