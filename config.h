#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <sys/types.h>

#define MAX_PROCESS 20
#define MAX_TIMEOUT 100
#define BUFFER_SIZE 400
#define msgsize 80
#define REQ 1


const key_t key_license = 2604;
const key_t key_childlist = 1708;
const key_t key_choosing = 4508;
const key_t key_number = 2223;

void logmsg(const char*);

#endif
