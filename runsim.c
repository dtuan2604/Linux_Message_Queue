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
		fprintf(stderr,"%s: ",programname);
		perror("Error");
	}
	return;
}
void del_shm(int shmid){
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if(ctl_return == -1){
		fprintf(stderr,"%s: ",programname);
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
        }else if(childPid == 0){
                int shmid;
                int *shm;
                int test;
                shmid = shmget(key, SHM_SIZE, 0666);
                if(shmid < 0){
                        fprintf(stderr,"%s: ",programname);
                        perror("Error");
                        exit(1);
                }
                shm = shmat(shmid, NULL, 0);
                if(shm == (int *) -1){
                        fprintf(stderr,"%s: ",programname);
                        perror("Error");
                        exit(1);
                }

                test = *shm;
                printf("Check share memory from child with process id %d: %d.\n",getpid(), test);
                dt_shm(shm);
		exit(0);
        }else{
                printf("\nI am a parent! My process id is %d, and my child is %d.\n",getpid(),childPid);
		waitpid(childPid, &status, 0);
        }
        return;

}

void runProcess(int nLicense){
	int shmid;
	int *shm;
	int test;
        shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

        if(shmid < 0){
        //        perror("shmget");
           	fprintf(stderr,"%s: ",programname);
                perror("Error:");
		exit(1);
        }

        shm = shmat(shmid, NULL, 0);

        if(shm == (int *) -1){
                fprintf(stderr,"%s: ",programname);
                perror("Error:");
                exit(1);
        }

        *shm = nLicense;
        test = *shm;
        printf("Check share memory: %d.\n", test);
	
	createChild();
	
        dt_shm(shm);
       	del_shm(shmid);

}
int main(int argc, char** argv){
	programname = argv[0];

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
	
	return EXIT_SUCCESS;


}
