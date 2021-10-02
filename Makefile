CLASSES= license.o 
CC = gcc
CXXFLAGS = -g -Wall -pthread
DEPS = license.c
all: runsim testsim

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

runsim : $(CLASSES)
	$(CC) -o $@ $^ $(CXXFLAGS) $@.c -lm

testsim : $(CLASSES)
	$(CC) -o $@ $^ $(CXXFLAGS) $@.c -lm

clean: 
	rm runsim testsim *.o
