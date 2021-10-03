#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "config.h"
//#include "license.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>

char* programname;

key_t key_license = LICENSE;
key_t key_childlist = CHILDLIST;
key_t key_choosing = CHOOSING;
key_t key_number = NUMBER;

pid_t *childList = NULL;
pid_t parentPid;

int *shared_license = NULL;
int *choosing = NULL;
int *number = NULL;

int shmid_license;
int shmid_childList;
int shmid_choosing;
int shmid_number;

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
void childProcess(int pIndex, char* command){
	int i = 0;
	int j = 0;
	char filename[10] = {"\0"};
	char sleeptime[5] = {"\0"};
	char repfactor[5] = {"\0"};
	//printf("%s", command);
	//exit(0);
	while(!isspace(command[i])){
		filename[i] = command[i];
		i++;
	}
	
	i++;
	while(!isspace(command[i])){
		sleeptime[j] = command[i]; 
		i++;
		j++;
	}

	i++;
	j = 0;
	while(command[i] != '\n' ){
		repfactor[j] = command[i];
		i++;
		j++;
	}
	execl(filename, filename, sleeptime, repfactor, NULL);	
	
	exit(EXIT_SUCCESS);
}
void killAllProcesses(){
	int i;
	for(i = 0; i < numofProcesses; i++){
		if(childList[i] != 0){
			printf("Terminate child %d\n", childList[i]);
			kill(childList[i], SIGKILL); 
		}
	}
}
void removePid(pid_t p){
	int i;
	for(i = 0; i < numofProcesses; i++){
		if(childList[i] == p){
			childList[i] = 0;
			break;
		}
	}
}
void getlicense(){
	pid_t p;
	if(*shared_license <= 0){
		p = wait(NULL);
		(*shared_license)++;
		removePid(p);	
	}
	(*shared_license)--;

}
void returnlicense(){
	pid_t p;
	if((p = waitpid(-1, NULL, WNOHANG)) != 0){
		(*shared_license)++;
		removePid(p);		
	}
}
int findEmptychild(){
	int i;
	for(i = 0; i < numofProcesses; i++){
		if(childList[i] == 0){
			return i;
		}
	}
	return -1;
}
void deallocateMemory(){
	if(shared_license != NULL){
      		dt_shm(shared_license);
                del_shm(shmid_license);
       	}
        if(childList != NULL){
        	dt_shm(childList);
        	del_shm(shmid_childList);
        }
	if(choosing != NULL){
		dt_shm(choosing);
		del_shm(shmid_choosing);
	}
	if(number != NULL){
		dt_shm(number);
		del_shm(shmid_number);
	}
}
void alarm_handler(int sig){
	if(getpid() == parentPid){
		printf("Alarm handler is triggered\n");
		killAllProcesses();
		deallocateMemory();
	}
	exit(1);
}
void interrupt_handler(int sig){
        if(getpid() == parentPid){
		printf("Interrupt handler is triggered\n");
        	killAllProcesses();
		deallocateMemory();
	}
	exit(1);
}
int initLicense(int nLicense){
	int shmid = shmget(key_license, sizeof(int), IPC_CREAT | 0666);

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
	*shared_license = nLicense;
	return shmid;
}
int initChildList(int numofProcesses){
	int i;
	int shmid = shmget(key_childlist, sizeof(pid_t) * numofProcesses, IPC_CREAT | 0666);
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

	for( i = 0; i< numofProcesses; i++){
		childList[i] = 0;
	}
	return shmid;
}
int initChoosingList(int numofProcesses){
	int shmid = shmget(key_choosing, sizeof(int)*numofProcesses, IPC_CREAT | 0666);
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
int initNumberList(int numofProcesses){
	int i;
	int shmid = shmget(key_number, sizeof(int)*numofProcesses, IPC_CREAT | 0666);
        if(shmid < 0){
                fprintf(stderr,"%s: failed to get id ",programname);
                perror("Error:");
                exit(1);
        }
        number = (int *) shmat(shmid, NULL, 0);
	if(number == (int *) -1){
                fprintf(stderr,"%s: failed to get pointer ",programname);
                perror("Error:");
                exit(1);
        }

	for( i = 0; i< numofProcesses; i++){
                number[i] = 0;
        }
	

        return shmid;


}
void initProcess(int nLicense){

	parentPid = getpid();
	numofProcesses = nLicense;

	shmid_license = initLicense(nLicense);
	shmid_childList = initChildList(numofProcesses);
	shmid_choosing = initChoosingList(numofProcesses);
	shmid_number = initNumberList(numofProcesses);	
	
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
		inputChar = getchar();
	}	
	
	char line[20];
	int status;
	int i = 0;
	int pIndex = 0;
	k = 0;
	while(commands[i] != '\0'){
		line[k] = commands[i];
		if(line[k] == '\n'){
			getlicense();
			pIndex = findEmptychild();
			pid_t childPid = fork();
			if(childPid < 0){
                		fprintf(stderr,"%s: ",programname);
                		perror("Error");
            		    	exit(1);
        		}else if(childPid == 0 ){
		 		childProcess(pIndex, line);
				break;
			}else{
				childList[pIndex] = childPid;
				i++;
				k = 0;
				int a;
				for(a = 0; a < 20; a++)
					line[a] = '\0';
				returnlicense();
			}
		}else{
			i++;
			k++;
		}
	}
	while(wait(&status) > 0); 
			
        dt_shm(shared_license);
       	dt_shm(childList);
	dt_shm(choosing);
	dt_shm(number);

	del_shm(shmid_license);
	del_shm(shmid_childList);
	del_shm(shmid_choosing);
	del_shm(shmid_number);

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
