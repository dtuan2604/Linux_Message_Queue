CLASSES= 
CC = gcc
CXXFLAGS = -g -Wall -pthread

all: runsim testsim

runsim : $(CLASSES)
	$(CC) -o $@ $^ $(CXXFLAGS) $@.c -lm

testsim : $(CLASSES)
	$(CC) -o $@ $^ $(CXXFLAGS) $@.c -lm

clean: 
	rm runsim testsim
