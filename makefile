CC = g++
CFLAGS = -lpthread

all: sensor monitor

sensor: sensor.cpp
	$(CC) sensor.cpp -o sensor $(CFLAGS)

monitor: monitor.cpp
	$(CC) monitor.cpp -o monitor $(CFLAGS)