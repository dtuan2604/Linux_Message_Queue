#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int sleeptime;
char* repfactor;
char* programname;
char msg[70];
char *text = NULL;
#define msgsize 70

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

void assignmsg(int currIterate){
	int k;
	for(k = 0; k<msgsize; k++){
		msg[k] = '\0';
	}
	
	char i[5];
	sprintf(i,"%d",currIterate); 
	
	char pid[10];
	sprintf(pid,"%d",getpid());
	
	time_t t = time(NULL);
	struct tm * local;
	local = localtime(&t);
	char ctime[30];
	strcpy(ctime,asctime(local));
	ctime[strlen(ctime)-1] = '\0';
	
	strcat(msg, ctime);
	strcat(msg,"\t");
	strcat(msg, pid);
        strcat(msg,"\t");
	strcat(msg, "Iteration #");
	strcat(msg, i); 
	strcat(msg, " of ");
	strcat(msg, repfactor);
	strcat(msg,"\n");
	return;

}
void generateLog(){
	if((text = (char*) malloc(BUFFER_LOG)) == NULL){
		fprintf(stderr,"%s: failed to allocate log. ",programname);
                perror("Error:");
                exit(1);
        }
	
	int reps = atoi(repfactor);
        int i;
	for(i = 0; i < reps; i++){
                sleep(sleeptime);
                assignmsg(i);
                strcat(text, msg);
        }

}
void logmsg(const char * msg){
	FILE * ptr = fopen(LOGFILE, "a");
	if(ptr == NULL){
		fprintf(stderr,"%s: ",programname);
                perror("Error:");
                exit(1);	
	}
	fputs(msg, ptr);
	fclose(ptr);
	free(text);
	return;

}
void handler(){
	if(text != NULL){
		free(text);
	}
	exit(1);
}
int main(int argc, char** argv){
	programname = argv[0];
	signal(SIGINT, handler);
	signal(SIGALRM, handler);
	if(argc != 3){
		fprintf(stderr, "ERROR: Please pass in two positive integer number!\n");
		return EXIT_FAILURE;
	}else{
		if((validNum(argv[1]) == 1) && (validNum(argv[2]) == 1)){
			sleeptime = atoi(argv[1]);
			repfactor = argv[2];
		}else{
			fprintf(stderr, "ERROR: Please pass in two positive integer number!\n");
			return EXIT_FAILURE;
		}

	}
	
	generateLog();

	logmsg(text);

	return EXIT_SUCCESS;





}
