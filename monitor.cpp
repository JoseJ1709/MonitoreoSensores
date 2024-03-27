//
// Created by jose-jaramillo on 3/26/24.
//
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "buffer.cpp"


struct ThreadArgs {
    Buffer* bufferPh;
    Buffer* bufferTemp;
    char* pipeName;
    bool isFinished;
};
// Thread functions
void* h_recolector(void* arg) {

    ThreadArgs* args = (ThreadArgs*)arg;
    Buffer* bufferPh = args->bufferPh;
    Buffer* bufferTemp = args->bufferTemp;
    char* pipeName = args->pipeName;

    // Open pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        args->isFinished = true;
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return NULL;
    }

    // Read data from pipe and add to buffers
    int data;
    while (read(pipeFd, &data, sizeof(data)) > 0) {
        if (data >= 0) {
            if (20>= data <= 31.6) {  // Assuming PH values are between 0 and 100
                bufferPh->add(data);
            } else {
                bufferTemp->add(data);
            }
        } else {
            std::cerr << "Error: received negative value from sensor" << std::endl;
        }
    }

    // Close pipe
    close(pipeFd);

    return NULL;
}

void* h_ph(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Buffer* bufferPh = args->bufferPh;
    // Open file
    std::ofstream filePh("file-ph.txt");
    if (!filePh.is_open()) {
        std::cerr << "Failed to open file: file-ph.txt" << std::endl;
        return NULL;
    }
    if(args->isFinished){
        filePh.close();
        return NULL;
    }

    // Write data from buffer to file
    int data;
    while ((data = bufferPh->remove()) != -1) {  // Assuming -1 indicates end of data
        filePh << data << " " << time(0) << std::endl;  // Append current time
    }

    // Close file
    filePh.close();

    return NULL;
}

void* h_temperatura(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Buffer* bufferTemp = args->bufferTemp;
    // Open file
    std::ofstream fileTemp("file-temp.txt");
    if (!fileTemp.is_open()) {
        std::cerr << "Failed to open file: file-temp.txt" << std::endl;
        return NULL;
    }
    if (args->isFinished){
        fileTemp.close();
        return NULL;
    }
    // Write data from buffer to file
    int data;
    while ((data = bufferTemp->remove()) != -1) {  // Assuming -1 indicates end of data
        fileTemp << data << " " << time(0) << std::endl;  // Append current time
    }

    // Close file
    fileTemp.close();

    return NULL;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int opt;
    int bufferSize = 0;
    char* fileTemp = nullptr;
    char* filePh = nullptr;
    char* pipeName = nullptr;

    while ((opt = getopt(argc, argv, "b:t:h:p:")) != -1) {
        switch (opt) {
            case 'b':
                bufferSize = atoi(optarg);
                break;
            case 't':
                fileTemp = optarg;
                break;
            case 'h':
                filePh = optarg;
                break;
            case 'p':
                pipeName = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -b bufferSize -t fileTemp -h filePh -p pipeName" << std::endl;
                return 1;
        }
    }

    // Open pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return 1;
    }

    // Create buffers
    Buffer bufferPh(bufferSize);
    Buffer bufferTemp(bufferSize);

    ThreadArgs args;
    args.bufferPh = &bufferPh;
    args.bufferTemp = &bufferTemp;
    args.pipeName = pipeName;
    // Create threads
    pthread_t threadRecolector, threadPh, threadTemp;
    pthread_create(&threadRecolector, NULL, h_recolector, &args);
    pthread_create(&threadPh, NULL, h_ph, &args);
    pthread_create(&threadTemp, NULL, h_temperatura, &args);

    // Join threads
    pthread_join(threadRecolector, NULL);
    pthread_join(threadPh, NULL);
    pthread_join(threadTemp, NULL);

    // Close pipe
    close(pipeFd);

    return 0;
}