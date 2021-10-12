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

struct message{
	long mtype;
	char mtext[10];
} msg;

char* programname;
char* terminateLog = NULL;

pid_t *childList = NULL;
pid_t parentPid;

int *shared_license = NULL;
int *choosing = NULL;
int *number = NULL;

int msgid = -1;
int shmid_license;
int shmid_childList;
int shmid_choosing;
int shmid_number;

int numofLine = 0;
int numofProcesses = 0; 
int nLicense;

void deallocateMemory();
void addTerminateLog(pid_t);
void dt_shm(int*);
void del_shm(int);
void del_queue(int);
void killAllProcesses();

void alarm_handler();
void interrupt_handler();

void initTerminationLog();
int initLicense();
int initChildList(int);
int initChoosingList(int);
int initNumberList(int);

void getlicense();
void returnlicense();
int findEmptychild();
void removePid(pid_t);


void childProcess(int, char*);
void parentProcess();

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
			if(inRange(argv[1], MAX_PROCESS) == 0){
				fprintf(stderr, "%s: ERROR: Please enter the number of license as a number smaller than %d.\n",programname,MAX_PROCESS);
				return EXIT_FAILURE;
			}else{
				nLicense = atoi(argv[1]);
				printf("The number of license is: %d.\n", nLicense);		
			}
		}else{
			fprintf(stderr, "ERROR: Please input a positive integer number of license.\n");
			return EXIT_FAILURE;
		}
	}
	
	parentProcess();
	
	printf("The program run successfully. It is now ending.\n");
	sleep(1); 
	return EXIT_SUCCESS;


}
void parentProcess(){

        parentPid = getpid();
        numofProcesses = nLicense;

        shmid_license = initLicense();
        shmid_childList = initChildList(numofProcesses);
        shmid_choosing = initChoosingList(numofProcesses);
        shmid_number = initNumberList(numofProcesses);
        initTerminationLog();

        char line[MAX_CANON];
        int pIndex;
        pid_t p;

        while(fgets(line, MAX_CANON, stdin) != NULL){
                numofLine++;
                getlicense();
                pIndex = findEmptychild();
                pid_t childPid = fork();
                if(childPid < 0){
                        fprintf(stderr,"%s: ",programname);
                        perror("Error");
                        exit(1);
                }else if(childPid == 0 ){
                        childProcess(pIndex, line);
                }else{
                        childList[pIndex] = childPid;
                        if((p = waitpid(-1, NULL, WNOHANG)) != 0){
                                nLicense++;
                                addTerminateLog(p);
                                removePid(p);
                        }
                }

        }

        while((p =  wait(NULL)) > 0){
                addTerminateLog(p);
        }

        logmsg(terminateLog);

        deallocateMemory();
}
void childProcess(int index, char* command){
        int i = 0;
        int j = 0;
        char filename[10] = {"\0"};
        char sleeptime[5] = {"\0"};
        char repfactor[5] = {"\0"};
        char pIndex[3] = {"\0"};
        
	snprintf(pIndex, 10, "%d", index);
        
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

        pid_t grandChild = fork();
        if(grandChild < 0){
                fprintf(stderr,"%s: ",programname);
                perror("Error");
                exit(1);
        }else if(grandChild == 0){
                execl(filename, filename, sleeptime, repfactor, pIndex, NULL);
        }else{
                waitpid(grandChild,NULL,WUNTRACED);
                returnlicense();
        }
        exit(EXIT_SUCCESS);
}
int initLicense(){
        int i, rc;
        msg.mtype = REQ;
        strcpy(msg.mtext,"req");

        msgid = msgget(key_license, 0666 | IPC_CREAT);
        if (msgid == -1) {
                fprintf(stderr,"%s: failed to get queue id. ",programname);
                perror("msgget");
                exit(1);
        }

        for(i = 0; i<nLicense; i++){
                if((rc = msgsnd(msgid, &msg, sizeof(msg), 0)) == -1){
                        fprintf(stderr,"%s: failed to send message. ",programname);
                        perror("msgget");
                        exit(1);

                }
        }
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
void getlicense(){
        pid_t p;
        if(nLicense <= 0){
                p = wait(NULL);
                addTerminateLog(p);
                nLicense++;
                removePid(p);
        }
        int rc = msgrcv(msgid, &msg, sizeof(msg), REQ, 0);
        if(rc == -1){
                fprintf(stderr,"%s: failed to send message. ",programname);
                perror("msgget");
                exit(1);
        }
        fflush(stdout);
        nLicense--;

}
void returnlicense(){
        int rc;
        if((rc = msgsnd(msgid, &msg, sizeof(msg), 0)) == -1){
               fprintf(stderr,"%s: failed to send message. ",programname);
               perror("msgget");
               exit(1);
        }
}
void initTerminationLog(){
        if((terminateLog = (char*)malloc(2)) == NULL){
                fprintf(stderr,"%s: failed to allocate log. ",programname);
                perror("Error:");
                exit(1);
        }
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
        if(msgid != -1){
                del_queue(msgid);
        }
        if(terminateLog != NULL)
                free(terminateLog);
}
void del_queue(int queueID){
        int ctl_return = msgctl(queueID, IPC_RMID, NULL);
        if(ctl_return == -1){
                fprintf(stderr,"%s: failed to delete queue %d. ",programname,queueID);
                perror("Error");
        }
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
void addTerminateLog(pid_t p){
        int i;
        char msg[msgsize];
        char* temp = realloc(terminateLog, msgsize * numofLine);
        if(temp == NULL){
                fprintf(stderr,"%s: failed to allocate log. ",programname);
                perror("Error:");
                exit(1);
        }else
                terminateLog = temp;
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
int findEmptychild(){
        int i;
        for(i = 0; i < numofProcesses; i++){
                if(childList[i] == 0){
                        return i;
                }
        }
        return -1;
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

