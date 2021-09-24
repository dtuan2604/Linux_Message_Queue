#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



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



int main(int argc, char** argv){
	int sleep;
	int repfactor;


	if(argc != 3){
		fprintf(stderr, "ERROR: Please pass in two positive integer number!\n");

	}else{
		if((validNum(argv[1]) == 1) && (validNum(argv[2]) == 1)){
			sleep = atoi(argv[1]);
			repfactor = atoi(argv[2]);
			printf("The sleep time: %d.\n", sleep);
			printf("The repeat factor: %d.\n", repfactor);
		}else{
			fprintf(stderr, "ERROR: Please pass in two positive integer number!\n");
			return EXIT_FAILURE;
		}

	}





	return EXIT_SUCCESS;





}
