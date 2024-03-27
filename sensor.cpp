//
// Created by jose-jaramillo on 3/26/24.
//
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int opt;
    int sensorType = 0;
    int timeInterval = 0;
    char* fileName = nullptr;
    char* pipeName = nullptr;

    while ((opt = getopt(argc, argv, "s:t:f:p:")) != -1) {
        switch (opt) {
            case 's':
                sensorType = atoi(optarg);
                break;
            case 't':
                timeInterval = atoi(optarg);
                break;
            case 'f':
                fileName = optarg;
                break;
            case 'p':
                pipeName = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -s sensorType -t timeInterval -f fileName -p pipeName" << std::endl;
                return 1;
        }
    }
    // Create pipe
    if (mkfifo(pipeName, 0666) < 0) {
        std::cerr << "Failed to create pipe: " << pipeName << std::endl;
        return 1;
    }

    // Open data file
    std::ifstream dataFile(fileName);
    if (!dataFile.is_open()) {
        std::cerr << "Failed to open data file: " << fileName << std::endl;
        return 1;
    }

    // Open pipe
    int pipeFd = open(pipeName, O_WRONLY);
    if (pipeFd < 0) {
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return 1;
    }

    // Read data from file and write to pipe in a loop
    std::string line;
    while (std::getline(dataFile, line)) {
        write(pipeFd, line.c_str(), line.size() + 1);
        sleep(timeInterval);
    }

    // Close file and pipe
    dataFile.close();
    close(pipeFd);

    return 0;
}