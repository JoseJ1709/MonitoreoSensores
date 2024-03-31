//
// Created by jose-jaramillo on 3/26/24.
//
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include "buffer.cpp"


struct ThreadArgs {
    Buffer* bufferPh;
    Buffer* bufferTemp;
    char* pipeName;
    bool isFinished;
};

bool is_float(const std::string& str) {
    try {
        std::size_t pos;
        std::stof(str, &pos);
        // La conversión tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

// Función para verificar si una cadena representa un número entero
bool is_integer(const std::string& str) {
    try {
        std::size_t pos;
        std::stoi(str, &pos);
        // La conversión tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}
std::string getCurrentTime() {
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
    return std::string(timeString);
}
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

    char buffer[128];
    ssize_t bytesRead;
    while ((bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        std::string data(buffer);
        if (is_integer(data)) {
            int value = std::stoi(data);
            if (value >= 0) {
                bufferTemp->add(data);
            } else {
                std::cerr << "Error: received negative value from sensor" << std::endl;
            }
        }
        // Verificar si los datos son de tipo float
        else if (is_float(data)) {
            float value = std::stof(data);
            if (value >= 0.0) {
                bufferPh->add(data);
            } else {
                std::cerr << "Error: received negative value from sensor" << std::endl;
            }
        }
        else {
            std::cerr << "Error: received invalid value from sensor" << std::endl;
        }
            // Verificar si los datos son de tipo entero
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
        bufferPh->~Buffer();
        return NULL;
    }

    // Write data from buffer to file
    std::string data;
    while ((data = bufferPh->remove()) != "-1") {
        float value = std::stof(data);// Assuming -1 indicates end of data
        if(value >=8.0 || value <= 6.0){
            std::cout << "Alerta: el valor de ph es: " << value << std::endl;
        }
        filePh << value << " " << getCurrentTime() << std::endl;  // Append current time
    }

    // Close file
    filePh.close();
    bufferPh->~Buffer();

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
        bufferTemp->~Buffer();
        return NULL;
    }
    // Write data from buffer to file
    std::string data;
    while ((data = bufferTemp->remove()) != "-1") {  // Assuming -1 indicates end of data
        int value = std::stoi(data);
        if(value >=31.6 || value <= 20){
            std::cout << "Alerta: el valor de temperatura es: " << value << std::endl;
        }
        fileTemp << value << " " << getCurrentTime() << std::endl;  // Append current time
    }

    // Close file
    fileTemp.close();
    bufferTemp->~Buffer();
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