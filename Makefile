CLASSES = config.o 
OBJ = testsim.o runsim.o
CC = gcc
CXXFLAGS = -g -Wall -pthread
DEPS = config.c runsim.c testsim.c
all: runsim testsim

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

runsim : $(CLASSES) $(OBJ)
	$(CC) $(CXXFLAGS) -lm -o $@ $(CLASSES) $@.o

testsim : $(CLASSES) $(OBJ)
	$(CC) $(CXXFLAGS) -lm -o $@ $(CLASSES) $@.o

clean: 
	rm runsim testsim *.o
