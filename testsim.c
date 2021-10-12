#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int sleeptime;

pid_t *childList = NULL;
int *shared_license = NULL;
int *choosing = NULL;
int *number = NULL;

char* repfactor;
char* programname;
char msg[msgsize];
char *text = NULL;


int getSharedMemory(); //this function return the number of license

void assignmsg(int);
void generateLog();
int maxNumber(int);

void dt_shm(int*);
void del_shm(int);
void deallocateMemory();
void interrupt_handler();

int main(int argc, char** argv){
	programname = argv[0];
	int i;
	signal(SIGINT, interrupt_handler);
	
	if(argc < 3){
		fprintf(stderr, "%s: ERROR: Please pass in two positive integer number!\n", programname);

		return EXIT_FAILURE;
	}else{
		if((validNum(argv[1]) == 1) && (validNum(argv[2]) == 1)){
			sleeptime = atoi(argv[1]);
			repfactor = argv[2];
			i = atoi(argv[3]);
		}else{
			fprintf(stderr, "%s: ERROR: Please pass in two positive integer number!\n",programname);
			return EXIT_FAILURE;
		}

	}
	
	int numofProcesses = getSharedMemory();
	int j;
	if(i == -1){
		fprintf(stderr, "%s: ERROR: Cannot find the process ID\n",programname);
		return(EXIT_FAILURE);
	}	

	generateLog();

	do{
 		choosing[i] = 1; 
 		number[i] = 1 + maxNumber(numofProcesses);
 		choosing[i] = 0; 
 		for ( j = 0; j < numofProcesses; j++ ) { 
 			while ( choosing[j] ); // Wait if j happens to be choosing 
 			while ( (number[j] != 0) && ( number[j] < number[i] || (number[j] == number[i] && j < i) ) ); 
 		} 
 
		logmsg(text);//critical section 
 
 		number[i] = 0; 
 		break;

	}while ( 1 );

	deallocateMemory();

	return EXIT_SUCCESS;

}
int getSharedMemory(){
        int shmid1 = shmget(key_license, sizeof(int), 0666);
        if(shmid1 < 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }

        shared_license = (int *) shmat(shmid1, NULL, 0);
        if(shared_license == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer. ",programname);
                perror("Error");
                exit(1);
        }

        int shmid2 = shmget(key_childlist, sizeof(pid_t) * (*shared_license), 0666);
        if(shmid2 < 0){
                fprintf(stderr,"%s: failed to get id. ",programname);
                perror("Error");
                exit(1);
        }

        childList = (pid_t *) shmat(shmid2, NULL, 0);
        if(childList == (pid_t *) -1){
                fprintf(stderr,"%s: failed to get pointer. ",programname);
                perror("Error");
                exit(1);
        }

        int shmid3 = shmget(key_choosing, sizeof(int) * (*shared_license), 0666);
        if(shmid3 < 0){
                fprintf(stderr,"%s: failed to get id. ",programname);
                perror("Error");
                exit(1);
        }

        choosing = (int *) shmat(shmid3, NULL, 0);
        if(choosing == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer. ",programname);
                perror("Error");
                exit(1);
        }

        int shmid4 = shmget(key_number, sizeof(int) * (*shared_license), 0666);
        if(shmid4 < 0){
                fprintf(stderr,"%s: failed to get id. ",programname);
                perror("Error");
                exit(1);
        }

        number = (int *) shmat(shmid4, NULL, 0);
        if(number == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer. ",programname);
                perror("Error");
                exit(1);
        }
        int numofProcesses = *shared_license;
	return numofProcesses;
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
        int reps = atoi(repfactor);
        if((text = (char*) malloc(msgsize * reps)) == NULL){
                fprintf(stderr,"%s: failed to allocate log. ",programname);
                perror("Error:");
                exit(1);
        }

        int i;
        for(i = 0; i < reps; i++){
                sleep(sleeptime);
                assignmsg(i);
                strcat(text, msg);
        }

}
int maxNumber(int numofProcesses){
        int i;
        int max = number[0];

        for(i = 1; i < numofProcesses; i++){
                if(max > number[i])
                        max = number[i];
        }
        return max;
}
void dt_shm(int* shm){
        int dt_return = shmdt(shm);
        if(dt_return == -1){
                fprintf(stderr,"%s: failed to detach. ",programname);
                perror("Error");
        }
        return;
}
void del_shm(int shmid){
        int ctl_return = shmctl(shmid, IPC_RMID, NULL);
        if(ctl_return == -1){
                fprintf(stderr,"%s: failed to delete %d. ",programname,shmid);
                perror("Error");
        }
        return;
}
void deallocateMemory(){
	if(shared_license != NULL)
                dt_shm(shared_license);
        if(childList != NULL)
                dt_shm(childList);
        if(choosing != NULL)
                dt_shm(choosing);
        if(number != NULL)
                dt_shm(number);
        if(text != NULL)
                free(text);
}
void interrupt_handler(){
	deallocateMemory();
	exit(1);
}
