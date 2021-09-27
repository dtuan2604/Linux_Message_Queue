#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


char* programname;


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

void dtshm(int* shm){
	int dt_return = shmdt(shm);
	if(dt_return == -1){
		fprintf(stderr,"%s: ",programname);
		perror("Error:");
	}
	return;
}
void delshm(int shmid){
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if(ctl_return == -1){
		fprintf(stderr,"%s: ",programname);
                perror("Error:");
	}	
	return;
}

int main(int argc, char** argv){
	int nLicense;
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

	int shmid;  	
	key_t key;
	int *shm;
	int test;

	key = 2604;

	shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

	if(shmid < 0){
		perror("shmget");
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
	dtshm(shm);	
	delshm(shmid);

	return EXIT_SUCCESS;


}
