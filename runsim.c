#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include "config.h"
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>

char* programname;
char* terminateLog = NULL;

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
int nLicense;



int validNum(char* num){
        int size = strlen(num);
        int i = 0;
        while(i < size){
                if(!isdigit(num[i])){
                        fprintf(stderr, "%s: ERROR: Please enter a positive integer.\n", programname);
			return 0;
		}
                i++;
        }
        return 1;
}
int inRange(char* num){
	int number = atoi(num);
	if(number > MAX_PROCESS){
		fprintf(stderr, "%s: ERROR: Please enter the number of license as a number smaller than %d.\n",programname,MAX_PROCESS);
		return 0;
	}
	return 1;

}
void addTerminateLog(pid_t p){
	int i;
	char msg[msgsize];
	for(i = 0; i < msgsize; i++){
		msg[i] = '\0';
	}	

	time_t t = time(NULL);
        struct tm * local;
        local = localtime(&t);
        char ctime[30];
        strcpy(ctime,asctime(local));
        ctime[strlen(ctime)-1] = '\0';
	
	char pid[10];
	sprintf(pid,"%d",p);

	strcat(msg, ctime);
	strcat(msg, "\t");
	strcat(msg, "Process ID: ");
	strcat(msg, pid);
	strcat(msg, "\tStatus: Terminated\n");
	strcat(terminateLog, msg);
	
	return;
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
void childProcess(int pIndex, char* command){
	int i = 0;
	int j = 0;
	char filename[10] = {"\0"};
	char sleeptime[5] = {"\0"};
	char repfactor[5] = {"\0"};
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
			kill(childList[i], SIGKILL);
			addTerminateLog(childList[i]); 
		}
	}
	if(terminateLog != NULL)
		logmsg(terminateLog);
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
	if(nLicense <= 0){
		p = wait(NULL);
		addTerminateLog(p);
		nLicense++;
		removePid(p);	
	}
	nLicense--;

}
void returnlicense(){
	pid_t p;
	if((p = waitpid(-1, NULL, WNOHANG)) != 0){
		nLicense++;
		addTerminateLog(p);
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
	if(terminateLog != NULL)
		free(terminateLog);
}
void alarm_handler(){
	if(getpid() == parentPid){
		printf("The program has exceeded the maximum timeout. It is ending now.\n");
		killAllProcesses();
		deallocateMemory();
	}
	sleep(1);
	exit(1);
}
void interrupt_handler(){
        if(getpid() == parentPid){
		printf("The program is being closed.\n");
        	killAllProcesses();
		deallocateMemory();
	}
	sleep(1);
	exit(1);
}
void initTerminationLog(int numofLine){
	if((terminateLog = (char*)malloc(msgsize * numofLine)) == NULL){
		fprintf(stderr,"%s: failed to allocate log. ",programname);
                perror("Error:");
                exit(1);		
	}	
}
int initLicense(){
	int shmid = shmget(key_license, sizeof(int), IPC_CREAT | 0666);

        if(shmid< 0){
                fprintf(stderr,"%s: failed to get id. ",programname);
                perror("Error");
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
void runProcess(){

	parentPid = getpid();
	numofProcesses = nLicense;

	shmid_license = initLicense();
	shmid_childList = initChildList(numofProcesses);
	shmid_choosing = initChoosingList(numofProcesses);
	shmid_number = initNumberList(numofProcesses);	
	
	char commands[BUFFER_SIZE];
	int k;
	int numofLine = 0;
	for(k = 0; k <BUFFER_SIZE; k++)
		commands[k] = '\0';	
	char inputChar;
	inputChar = getchar();
	while(inputChar != EOF){
		if(inputChar == '.')
			numofLine++;
		strncat(commands, &inputChar,1);
		inputChar = getchar();
	}	
	
	initTerminationLog(numofLine);
	
	char line[20];
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
		}else if(k >= MAX_CANON){
			fprintf(stderr,"%s: Error: The number of character has exceeded %d\n",programname, MAX_CANON);
			interrupt_handler();			
		}else{
			i++;
			k++;
		}
	}
	
	pid_t p;
	while((p =  wait(NULL)) > 0){
		addTerminateLog(p);
	}	
	
	logmsg(terminateLog);
		
        dt_shm(shared_license);
       	dt_shm(childList);
	dt_shm(choosing);
	dt_shm(number);

	del_shm(shmid_license);
	del_shm(shmid_childList);
	del_shm(shmid_choosing);
	del_shm(shmid_number);

	free(terminateLog);

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
	
	runProcess();
	printf("The program run successfully. It is now ending.\n");
	sleep(1); 
	return EXIT_SUCCESS;


}
