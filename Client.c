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

#define max_size 4096
#define N 50
char dir[N];
int main(int argc, char *argv[]){

	if(argc < 2){
		printf("Usage: %s path_to_file", argv[0]);
		exit (-1);
	}

	if (getcwd(dir, N) == NULL) {
		printf("getcwd error\n");
			exit(errno);
	}
	char path[N];
	
	sprintf(path, "%s/for_yulya.txt", dir);

	struct stat *about_file = (struct stat *) malloc (sizeof(struct stat));
	if (!about_file){
		printf("malloc error\n");
		exit(errno);
	}
	if ( stat(argv[1], about_file) == -1){
		printf("stat error\n");
		exit(errno);
	}
	long int file_size = about_file->st_size;

        key_t shm_key;
	key_t sem_key;
	struct sembuf sops[5];
        if ( (shm_key = ftok(path, 1)) == -1){
		printf("ftok error\n");
		exit (errno);
       	}

	if ( (sem_key = ftok(path, 6)) == -1){
		printf("ftok error\n");
		exit (errno);
       	}

       	int shm_id;
       	if ( (shm_id =  shmget(shm_key, max_size, IPC_CREAT|0666)) == -1){
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
	int fd;
	if (( fd = open (argv[1], O_RDONLY) ) == -1) {
		printf("error open\n");
		exit (errno);
	}
	while(1){
	
		sops[0].sem_num = 0;
		sops[0].sem_op = -1;
		sops[0].sem_flg = 0;

		sops[1].sem_num = 1;
		sops[1].sem_op = 1;
		sops[1].sem_flg = 0;

		sops[2].sem_num = 1;
		sops[2].sem_op = -1;
		sops[2].sem_flg = SEM_UNDO;

		sops[3].sem_num = 1;
		sops[3].sem_op = 0;
		sops[3].sem_flg = IPC_NOWAIT;


		if ( semop (sem_id, sops, 4) == -1 ){
			if ( shmctl( shm_id, 0, 0) == -1){
		       		printf("shmctl error\n");
				exit(errno);
		       	}
			
			if ( semctl( sem_id, 0, 0) == -1){
		       		printf("semctl error\n");
				exit(errno);
	       		}
			free(about_file);
			if ( remove(path) == -1){
				printf("remove file error\n");
				exit(errno);

			if (errno != EAGAIN)
				printf("semop2 error\n");
			exit(errno);
	       	}
		sleep(1);
		int rd = read(fd, buf, max_size);
		if (rd  != max_size){
			*(buf + rd) = '\0';
			exit(-1);			
		}	
		i++;
		sops[0].sem_num = 1;
		sops[0].sem_op = 1;
		sops[0].sem_flg = 0;	

		if ( semop (sem_id, sops, 1) == -1 ){
			printf("semop2 error\n");
			exit(errno);
	       	}

	}
}
