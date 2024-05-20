/*
Nombre del archivo: buffer.cpp
Autores: Lucas Rivera, Jose Jaramillo
Objetivo: Implementar los búferes para almacenar temporalmente las medidas de los sensores.
Módulos: add, remove
*/

#include "buffer.h" //librería que contiene la definición de la clase Buffer

/*
Nombre de la función: Buffer
Parámetros de entrada: int size
Valor que devuelve: N/A
Descripción: Constructor de la clase Buffer. Inicializa el mutex y las variables de condición.
*/
Buffer::Buffer(int size) : size(size){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condProducer, NULL);
    pthread_cond_init(&condConsumer, NULL);
}

/*Nombre de la función: ~Buffer
        Parámetros de entrada: N/A
        Valor que devuelve: N/A
        Descripción: Destructor de la clase Buffer. Destruye el mutex y las variables de condición.
*/
Buffer::~Buffer() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condProducer);
    pthread_cond_destroy(&condConsumer);
}

/*
Nombre de la función: add
Parámetros de entrada: std::string data
Valor que devuelve: void
Descripción: Añade un dato al búfer. Si el búfer está lleno, espera hasta que haya espacio.
*/
void Buffer::add(std::string data) {
    pthread_mutex_lock(&mutex);
    while (dataQueue.size() >= size) {
        pthread_cond_wait(&condProducer, &mutex);
    }
    dataQueue.push(data);
    pthread_cond_signal(&condConsumer);
    pthread_mutex_unlock(&mutex);
}

/*
Nombre de la función: remove
Parámetros de entrada: N/A
Valor que devuelve: std::string
Descripción: Elimina y devuelve un dato del búfer. Si el búfer está vacío, espera hasta que haya datos.
*/
std::string Buffer::remove() {
    pthread_mutex_lock(&mutex);
    while (dataQueue.empty()) {
        pthread_cond_wait(&condConsumer, &mutex);
    }
    std::string data = dataQueue.front();
    dataQueue.pop();
    pthread_cond_signal(&condProducer);
    pthread_mutex_unlock(&mutex);
    return data;
}