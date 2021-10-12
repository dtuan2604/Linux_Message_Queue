#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

int validNum(char* num){
        int size = strlen(num);
        int i = 0;
        while(i < size){
                if(!isdigit(num[i]))
                        return 0;
                i++;
        }
        return 1;
}
int inRange(char* num, int range){
        int number = atoi(num);
        if(number > range)
                return 0;
        return 1;

}
