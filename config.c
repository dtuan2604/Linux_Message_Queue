//#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#define LOGFILE "logmsg.txt"


void logmsg(const char * msg){
        FILE * ptr = fopen(LOGFILE, "a");
        if(ptr == NULL){
                perror("Error at license function:");
                exit(1);
        }
        fputs(msg, ptr);
        fclose(ptr);
        return;

}
