/*
Nombre del archivo: monitor.cpp
Autores: Lucas Rivera, Jose Jaramillo
Objetivo: Implementar el monitoreo de los sensores y la escritura de los datos en los archivos correspondientes.
Módulos: is_float, is_integer, getCurrentTime, h_recolector, h_ph, h_temperatura, main
*/
#include "buffer.cpp"
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>

/*
Nombre de la estructura: ThreadArgs
Descripción: Estructura para almacenar los argumentos que se pasarán a los hilos.
Contiene punteros a los búferes para los datos de pH y temperatura,
los nombres de los archivos donde se escribirán los datos,
el nombre del pipe para la comunicación con el sensor,
y un semáforo para la sincronización de los hilos.
*/
struct ThreadArgs {
    Buffer *bufferPh;      // Puntero al búfer para los datos de pH
    Buffer *bufferTemp;    // Puntero al búfer para los datos de temperatura
    char *pipeName;        // Nombre del pipe para la comunicación con el sensor
    char *fileTemp;        // Nombre del archivo donde se escribirán los datos de temperatura
    char *filePh;          // Nombre del archivo donde se escribirán los datos de pH
    sem_t sem;             // Semaforo para la sincronización de los hilos
};


/*
Nombre de la función: is_float
Parámetros de entrada: const std::string &str
Valor que devuelve: bool
Descripción: Verifica si una cadena representa un número de punto flotante.
*/
bool is_float(const std::string &str) {
    try {
        std::size_t pos;
        std::stof(str, &pos);
        // La conversión tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

/*
Nombre de la función: is_integer
Parámetros de entrada: const std::string &str
Valor que devuelve: bool
Descripción: Verifica si una cadena representa un número entero.
*/
bool is_integer(const std::string &str) {
    try {
        std::size_t pos;
        std::stoi(str, &pos);
        // La conversión tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

/*
Nombre de la función: getCurrentTime
Parámetros de entrada: N/A
Valor que devuelve: std::string
Descripción: Obtiene la hora actual.
*/
std::string getCurrentTime() {
    std::time_t currentTime = std::time(nullptr);
    std::tm *localTime = std::localtime(&currentTime);
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
    return std::string(timeString);
}

/*
Nombre de la función: h_recolector
Parámetros de entrada: void *arg
Valor que devuelve: void*
Descripción: Función para recolectar datos de los sensores y manejarlos entre Hilos.
*/
void *h_recolector(void *arg) {

    ThreadArgs *args = (ThreadArgs *)arg;
    Buffer *bufferPh = args->bufferPh;
    Buffer *bufferTemp = args->bufferTemp;
    char *pipeName = args->pipeName;

    // Abrir el Pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        sem_post(&args->sem);
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return NULL;
    }
    // Leer datos del pipe
    std::string line;
    while (true) {
        int attempts = 0;
        int bytesRead = 0;
        do {
            char buffer[256];
            bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                // Procesar la linea y agregar a los buffers
                buffer[bytesRead] = '\0';
                line = buffer;
                // Revisando si la linea es un entero
                if (is_integer(line)) {
                    int value = std::stoi(line);
                    if (value >= 0) {
                        bufferTemp->add(line);
                    } else {
                        std::cerr << "Error: received negative value from sensor"
                                  << std::endl;
                    }
                }
                    // Revisando si la linea es un flotante
                else if (is_float(line)) {
                    float value = std::stof(line);
                    if (value >= 0.0) {
                        bufferPh->add(line);
                    } else {
                        std::cerr << "Error: received negative value from sensor"
                                  << std::endl;
                    }
                } else {
                    std::cerr << "Error: received invalid value from sensor" << std::endl;
                }
                attempts = 0; // Reiniciar el contador de intentos

            } else {
                // El sensor no está conectado, esperar 1 segundo
                sleep(1);
                attempts++;
                std::cout << "Esperando sensor: " << attempts << " segundos"
                          << std::endl;
            }

        } while (bytesRead <= 0 && attempts < 10);

        if (attempts == 10) {
            // El sensor no se conectó después de 10 intentos, terminar el proceso
            // Enviando mensajes a los otros hilos para que terminen
            bufferPh->add("-1");
            bufferTemp->add("-1");
            // Borrando pipe y terminando el proceso
            unlink(args->pipeName);
            std::cout << "Finished processing measurements" << std::endl;
            break;
        }
    }
    // Cerrando el pipe
    close(pipeFd);

    return NULL;
}

/*
Nombre de la función: h_ph
Parámetros de entrada: void *arg
Valor que devuelve: void*
Descripción: Función para escribir los datos del sensor de pH en un archivo.
*/
void *h_ph(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    Buffer *bufferPh = args->bufferPh;
    char *fileName = args->filePh;

    int sem_val;
    sem_getvalue(&args->sem, &sem_val);
    // Abrir el archivo
    std::ofstream filePh(fileName);
    if (!filePh.is_open()) {
        std::cerr << "Failed to open file: file-ph.txt" << std::endl;
        return NULL;
    }

    if (sem_val > 0) {
        filePh.close();
        bufferPh->~Buffer();
        return NULL;
    }

    // Escritura de datos del buffer al archivo
    std::string data;
    while ((data = bufferPh->remove()) != "-1") {
        float value = std::stof(data);
        if (value >= 8.0 || value <= 6.0) {
            std::cout << "Alerta: el valor de ph es: " << value << std::endl;
        }
        filePh << value << " " << getCurrentTime() << std::endl;
    }

    // Cerrando archivo
    filePh.close();
    bufferPh->~Buffer();

    return NULL;
}

/*
Nombre de la función: h_temperatura
Parámetros de entrada: void *arg
Valor que devuelve: void*
Descripción: Función para escribir los datos del sensor de temperatura en un archivo.
*/
void *h_temperatura(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    Buffer *bufferTemp = args->bufferTemp;
    char *fileName = args->fileTemp;

    int sem_val;
    sem_getvalue(&args->sem, &sem_val);
    // Abrir el archivo
    std::ofstream fileTemp(fileName);
    if (!fileTemp.is_open()) {
        std::cerr << "Failed to open file: file-temp.txt" << std::endl;
        return NULL;
    }
    if (sem_val > 0) {
        fileTemp.close();
        bufferTemp->~Buffer();
        return NULL;
    }
    // Escribir datos del buffer al archivo
    std::string data;
    while ((data = bufferTemp->remove()) != "-1") {
        int value = std::stoi(data);
        if (value >= 31.6 || value <= 20) {
            std::cout << "Alerta: el valor de temperatura es: " << value << std::endl;
        }
        fileTemp << value << " " << getCurrentTime() << std::endl;
    }

    // Cerrando archivo
    fileTemp.close();
    bufferTemp->~Buffer();
    return NULL;
}

/*
Nombre de la función: main
Parámetros de entrada: int argc, char *argv[]
Valor que devuelve: int
Descripción: Función principal que inicia el monitoreo de los sensores.
*/
int main(int argc, char *argv[]) {
    // Iniciando variables

    int opt;
    int bufferSize = 0;
    char *fileTemp = nullptr;
    char *filePh = nullptr;
    char *pipeName = nullptr;
    // Revisando argumentos y asignandolos
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
                std::cerr << "Usage: " << argv[0]
                          << " -b bufferSize -t fileTemp -h filePh -p pipeName"
                          << std::endl;
                return 1;
        }
    }

    // Creando Pipe
    if (mkfifo(pipeName, 0666) < 0) {
        std::cerr << "Failed to create pipe: " << pipeName << std::endl;
        return 1;
    }

    // Abriendo pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return 1;
    }

    // Creando buffers
    Buffer bufferPh(bufferSize);
    Buffer bufferTemp(bufferSize);

    ThreadArgs args;
    args.bufferPh = &bufferPh;
    args.bufferTemp = &bufferTemp;
    args.pipeName = pipeName;
    args.fileTemp = fileTemp;
    args.filePh = filePh;
    sem_init(&args.sem, 0, 0);
    // Creando Hilos
    pthread_t threadRecolector, threadPh, threadTemp;
    pthread_create(&threadRecolector, NULL, h_recolector, &args);
    pthread_create(&threadPh, NULL, h_ph, &args);
    pthread_create(&threadTemp, NULL, h_temperatura, &args);
    // Uniendo Hilos
    pthread_join(threadRecolector, NULL);
    pthread_join(threadPh, NULL);
    pthread_join(threadTemp, NULL);

    // Cerrando pipe y destru-yendo semaforo
    close(pipeFd);
    sem_destroy(&args.sem);
    return 0;
}