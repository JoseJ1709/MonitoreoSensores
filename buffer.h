/*
Nombre del archivo: buffer.h
Autores: Lucas Rivera, Jose Jaramillo
Objetivo: Definir la clase Buffer y sus métodos.
Módulos: Buffer, ~Buffer, add, remove
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <queue>
#include <pthread.h>
#include <string>

class Buffer {
private:
    std::queue<std::string> dataQueue;
    pthread_mutex_t mutex;
    pthread_cond_t condProducer;
    pthread_cond_t condConsumer;
    int size;

public:
    Buffer(int size);
    ~Buffer();
    void add(std::string data);
    std::string remove();
};

#endif //BUFFER_H