#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <errno.h>

#define page_size 4096
#define N 256
#define size 12

char dir[N];
int main(int argc, char *argv[]){

	if (getcwd(dir, N) == NULL) {
		printf("getcwd error\n");
			exit(errno);
	}
	char path[N];
	
	sprintf(path, "%s/for_yulya.txt", dir);

	open(path, O_CREAT);

        key_t shm_key;
	key_t sem_key;
	struct sembuf sops[4];
        if ( (shm_key = ftok(path, 1)) == -1){
		printf("ftok error\n");
		exit (errno);
       	}

	if ( (sem_key = ftok(path, 6)) == -1){
		printf("ftok error\n");
		exit (errno);
       	}

       	int shm_id;
       	if ( (shm_id =  shmget(shm_key, page_size+size+1, IPC_CREAT|0666)) == -1){
		printf("shmget error\n");
		exit(errno);
       	}

       	char *buf;
        if ( (buf = (char *) shmat (shm_id, NULL, 0)) == (char *)(-1) ){
       		printf("shmat error\n");
		exit(errno);
       	}

	int sem_id;
	if ( (sem_id =  semget(sem_key, 3, IPC_CREAT|0666)) == -1){
		printf("semget error\n");
		exit(errno);
       	}
	int i = 0;

	sops[0].sem_num = 0;
	sops[0].sem_op = 1;
	sops[0].sem_flg = 0;
	if ( semop (sem_id, sops, 1) == -1){
		printf("semop1 error\n");
		exit(errno);
       	}


	while(1){
	
		sops[0].sem_num = 1;
		sops[0].sem_op = -1;
		sops[0].sem_flg = 0;  

		sops[1].sem_num = 0;
		sops[1].sem_op = 2;
		sops[1].sem_flg = 0;

		sops[2].sem_num = 0;
		sops[2].sem_op = -2;
		sops[2].sem_flg = SEM_UNDO;  

		sops[3].sem_num = 0;
		sops[3].sem_op = 0;
		sops[3].sem_flg = IPC_NOWAIT; 

		if ( semop (sem_id, sops, 4) == -1){

			if ( shmctl( shm_id, 0, 0) == -1){
		       		printf("shmctl error\n");
				exit(errno);
		       	}
			
			if ( semctl( sem_id, 0, 0) == -1){
		       		printf("semctl error\n"); 
				exit(errno);
	       		}

			if ( remove(path) == -1){
				printf("remove file error\n");
				exit(errno);
			}
			
			if (errno != EAGAIN){
				printf("semop1 error\n");
			}
			exit(errno);
	       	}	
		
		int need_size = 0;
		int two_power = 1;
		for (int i = 1; i<=12; i++){
			if ( *(buf+size-i) == '1'){
				need_size += two_power;
			}
			two_power = two_power * 2;
		}
		if ((write(1, buf+size, need_size)) == -1) {
			printf("write error\n");
			exit(errno);
		}
 
		sops[0].sem_num = 0;
		sops[0].sem_op = 1;
		sops[0].sem_flg = 0;

		if ( semop (sem_id, sops, 1) == -1){
			printf("semop2 error\n");
			exit(errno);
	       	}
	}
		
}
