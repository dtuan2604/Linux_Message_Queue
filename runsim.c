#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "config.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>


char* programname;

key_t key_license = 2604;
key_t key_pidlist = 1708;
pid_t *childList;
pid_t parentPid;

int *shared_license;
int shmid_license;
int shmid_childList;
int numofProcesses = 0; 


int validNum(char* num){
        int size = strlen(num);
        int i = 0;
        while(i < size){
                if(!isdigit(num[i])){
                        fprintf(stderr, "ERROR: Please enter a positive integer.\n");
			return 0;
		}
                i++;
        }
        return 1;
}
int inRange(char* num){
	int number = atoi(num);
	if(number > MAX_PROCESS){
		fprintf(stderr, "ERROR: Please enter the number of license as a number smaller than %d.\n", MAX_PROCESS);
		return 0;
	}
	return 1;

}

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
void childProcess(int i, char* command){
	printf("I am %d\n, I am taking command: %s.", getpid(), command);
	exit(0);
}
void killAllProcesses(){
	int i;
	if(getpid() == parentPid){
		for(i = 0; i < numofProcesses; i++){
		kill(childList[i], SIGKILL); 
		}
	}
}
void alarm_handler(){
	printf("Alarm handler is triggered\n");
        killAllProcesses();
        dt_shm(shared_license);
        dt_shm(childList);
        del_shm(shmid_license);
        del_shm(shmid_childList);

}
void interrupt_handler(){
        if(getpid() == parentPid){
		printf("Interrupt handler is triggered\n");
        	killAllProcesses();
		dt_shm(shared_license);
        	dt_shm(childList);
        	del_shm(shmid_license);
        	del_shm(shmid_childList);
	}
	exit(1);
}
int initLicense(int nLicense){
	int shmid = shmget(key_license, SHM_SIZE, IPC_CREAT | 0666);

        if(shmid< 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }

        shared_license = shmat(shmid, NULL, 0);

        if(shared_license == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }
	return shmid;
}
int initChildList(int numofProcesses){
	int i;
	int shmid = shmget(key_pidlist, sizeof(pid_t) * numofProcesses, IPC_CREAT | 0666);
	if(shmid < 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }
	
	childList = (pid_t *) shmat(shmid, NULL, 0);
	if(childList == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }

	for( i = 0; i< numofProcesses; i++){
		childList[i] = 0; 
	}
	return shmid;
}
void initProcess(int nLicense){

	parentPid = getpid();
	shmid_license = initLicense(nLicense);
	
	char* commands = NULL;
	if((commands = (char*) malloc(BUFFER_SIZE)) == NULL){
		fprintf(stderr,"%s: failed to get commands ",programname);
                perror("Error:");
                exit(1);
	} 	
	int k;
	for(k = 0; k <BUFFER_SIZE; k++)
		commands[k] = '\0';	
	char inputChar;
	inputChar = getchar();
	while(inputChar != EOF){
		strncat(commands, &inputChar,1);
		if(inputChar == '.')
			numofProcesses++;
		inputChar = getchar();
	}	
	
	shmid_childList = initChildList(numofProcesses);
	
	char line[20];
	int status;
	int i = 0;
	k = 0;
	while(commands[i] != '\0'){
		line[k] = commands[i];
		if(line[k] == '\n'){
			pid_t childPid = fork();
			if(childPid < 0){
                		fprintf(stderr,"%s: ",programname);
                		perror("Error");
            		    	exit(1);
        		}else if(childPid == 0 ){
		 		childProcess(i, line);
				break;
			}else{
				i++;
				k = 0;
				int a;
				for(a = 0; a < 20; a++)
					line[a] = '\0';
			}
		}else{
			i++;
			k++;
		}
	}
	while(wait(&status) > 0); 
			
        dt_shm(shared_license);
       	dt_shm(childList);
	del_shm(shmid_license);
	del_shm(shmid_childList);
	
	free(commands);

}
int main(int argc, char** argv){
	int nLicense;
	programname = argv[0];
	signal(SIGINT, interrupt_handler);
	signal(SIGALRM, alarm_handler);
	alarm(MAX_TIMEOUT);
	if((argc != 2) ){
		fprintf(stderr, "ERROR: Please input only one positive integer number as an argument.\n");
		return EXIT_FAILURE;	
	}else{
		if(validNum(argv[1]) == 1){
			if(inRange(argv[1]) == 0)
				return EXIT_FAILURE;
			else{
				nLicense = atoi(argv[1]);
				printf("The number of license is: %d.\n", nLicense);		
			}
		}else
			return EXIT_FAILURE;
	}
	
	initProcess(nLicense); 
	printf("\nCheckpoint\n");	
	return EXIT_SUCCESS;


}
