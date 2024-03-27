//
// Created by jose-jaramillo on 3/26/24.
//
#include "buffer.h"

Buffer::Buffer(int size) : size(size){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condProducer, NULL);
    pthread_cond_init(&condConsumer, NULL);
}

Buffer::~Buffer() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condProducer);
    pthread_cond_destroy(&condConsumer);
}

void Buffer::add(int data) {
    pthread_mutex_lock(&mutex);
    while (dataQueue.size() >= size) {
        pthread_cond_wait(&condProducer, &mutex);
    }
    dataQueue.push(data);
    pthread_cond_signal(&condConsumer);
    pthread_mutex_unlock(&mutex);
}

int Buffer::remove() {
    pthread_mutex_lock(&mutex);
    while (dataQueue.empty()) {
        pthread_cond_wait(&condConsumer, &mutex);
    }
    int data = dataQueue.front();
    dataQueue.pop();
    pthread_cond_signal(&condProducer);
    pthread_mutex_unlock(&mutex);
    return data;
}