//
// Created by jose-jaramillo on 3/26/24.
//
#ifndef BUFFER_H
#define BUFFER_H

#include <queue>
#include <pthread.h>

class Buffer {
private:
    std::queue<int> dataQueue;
    pthread_mutex_t mutex;
    pthread_cond_t condProducer;
    pthread_cond_t condConsumer;
    int size;

public:
    Buffer(int size);
    ~Buffer();
    void add(int data);
    int remove();
};

#endif //BUFFER_H