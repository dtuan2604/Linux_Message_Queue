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


int nLicense;
char* programname;
key_t key = 2604;
pid_t pidList;
pid_t parentPid;
int *shm_ptr;
int shm_id;


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
		fprintf(stderr,"%s: failed to delete ",programname);
                perror("Error at delete function");
	}	
	return;
}
void createChild(){
        pid_t childPid = fork();
        int status; 
	if(childPid < 0){
                fprintf(stderr,"%s: ",programname);
                perror("Error");
                exit(1);
        }else if(childPid == 0 ){
        	pidList = getpid();
		int shmid;
                int *shm;
                int test;
                shmid = shmget(key, SHM_SIZE, 0666);
                if(shmid < 0){
                        fprintf(stderr,"%s: failed to get id ",programname);
                        perror("Error");
                        exit(1);
                }
                shm = shmat(shmid, NULL, 0);
                if(shm == (int *) -1){
                        fprintf(stderr,"%s: failed to get pointer ",programname);
                        perror("Error");
                        exit(1);
                }
	
                test = *shm;
                printf("Check share memory from child with process id %d: %d.\n",getpid(), test);
                //while(1); // Test Timeout handler
		dt_shm(shm);
		exit(EXIT_SUCCESS);
        }else{
		waitpid(childPid, &status, 0);
   	}
        return;

}
void killAllProcesses(){
	kill(pidList, SIGKILL);
	exit(EXIT_SUCCESS); 

}
void alarm_handler(){
	printf("Alarm handler is triggered\n");
        dt_shm(shm_ptr);
        del_shm(shm_id);
	killAllProcesses();
}
void interrupt_handler (){
        if(getpid() == parentPid){
		printf("Interrupt handler is triggered\n");
        	dt_shm(shm_ptr);
        	del_shm(shm_id);
		killAllProcesses();
	}
}
void runProcess(int nLicense){
        int numofProcesses = 0;

	parentPid = getpid();
	shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

        if(shm_id < 0){
        //        perror("shmget");
           	fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
		exit(1);
        }

        shm_ptr = shmat(shm_id, NULL, 0);

        if(shm_ptr == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }

        *shm_ptr = nLicense; //assign number of license to the shared memory
	
	char* commands;
	if((commands = (char*) malloc(BUFFER_SIZE)) == NULL){
		fprintf(stderr,"%s: failed to get commands ",programname);
                perror("Error:");
                exit(1);

	} 	
	char inputChar;
	inputChar = getchar();
	while(inputChar != EOF){
		strncat(commands, &inputChar,1);
		if(inputChar == '.')
			numofProcesses++;
		inputChar = getchar();

	}	
	printf("Commands: %s\n",commands);
	printf("Number of commands: %d",numofProcesses);
	free(commands);
	exit(0);
		
	createChild();
	
        dt_shm(shm_ptr);
       	del_shm(shm_id);

}
int main(int argc, char** argv){
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
	
	runProcess(nLicense); 
	printf("Checkpoint");	
	return EXIT_SUCCESS;


}
