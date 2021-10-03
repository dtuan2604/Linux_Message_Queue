#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "license.h"
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int sleeptime;
int shmid_license;
int shmid_childList;
int shmid_choosing;
int shmid_number; 


pid_t *childList = NULL;
int *shared_license = NULL;
int *choosing = NULL;
int *number = NULL;

char* repfactor;
char* programname;
char msg[70];
char *text = NULL;

#define msgsize 70

void dt_shm(int* shm){
        int dt_return = shmdt(shm);
        if(dt_return == -1){
                fprintf(stderr,"%s: failed to detach ",programname);
                perror("Error");
        }
        return;
}
void del_shm(int shmid){
        int ctl_return = shmctl(shmid, IPC_RMID, NULL);
        if(ctl_return == -1){
                fprintf(stderr,"%s: failed to delete %d, ",programname,shmid);
                perror("Error");
        }
        return;
}
int getSharedLicense(){
	int shmid = shmget(key_license, sizeof(int), 0666);
	if(shmid< 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }

        shared_license = (int *) shmat(shmid, NULL, 0);
	if(shared_license == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }

	return shmid;
}
int getChildList(){
	int shmid = shmget(key_childlist, sizeof(pid_t) * (*shared_license), 0666);
	if(shmid < 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }

	childList = (pid_t *) shmat(shmid, NULL, 0);
        if(childList == (pid_t *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }
	return shmid;	
}
int getChoosingList(){
	int shmid = shmget(key_choosing, sizeof(int) * (*shared_license), 0666);
        if(shmid < 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }

        choosing = (int *) shmat(shmid, NULL, 0);
        if(choosing == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }
        return shmid;


}
int getNumberList(){
	int shmid = shmget(key_number, sizeof(int) * (*shared_license), 0666);
        if(shmid < 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error");
                exit(1);
        }

        number = (int *) shmat(shmid, NULL, 0);
        if(number == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }
        return shmid;

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
void handler(){
	if(text != NULL){
		free(text);
	}
	exit(1);
}
int main(int argc, char** argv){
	programname = argv[0];
	
	signal(SIGINT, handler);
	
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
	
	printf("I am %d, my commands is %d %s\n", getpid(), sleeptime,repfactor);
	shmid_license = getSharedLicense();	
	shmid_childList = getChildList();
	shmid_choosing = getChoosingList();
	shmid_number = getNumberList();
	return EXIT_SUCCESS;
	generateLog();

	logmsg(text);
	free(text);

	return EXIT_SUCCESS;





}
