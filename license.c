#include "license.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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



