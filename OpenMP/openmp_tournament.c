#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <stdbool.h>

#define WINNER 0
#define LOSER 1
#define BYE 2
#define CHAMPION 3
#define DROPOUT 4

typedef struct record{
    int role;
    int opponent;
    int flag;
}record_t;

record_t** players;
int P, N;

void barrier_init()
{
	printf("In barrier init\n");
	int i, k;
	int rounds = (int)(log2(P));
	players = (record_t**)malloc(P * sizeof(record_t*));
	for(i = 0; i < P; i++)
	{
		players[i] = (record_t*)calloc(rounds+1, sizeof(record_t));
	}
	for(i = 0; i < P; i++)
	{
		printf("init processor i %d\n", i);
		for(k=0;k<=rounds;k++)
		{
			printf("init round k %d\n", k);
			//Initializing flag
			players[i][k].flag = 0;
			
			//Initializing role and opponent
			if (k==0){
				players[i][k].role = DROPOUT;
				players[i][k].opponent = -1;
			}
			else if (k>0){
				if (i == 0 && (1<<k)>=P){
					players[i][k].role = CHAMPION;
					players[i][k].opponent = i+(1<<(k-1));
				}
				else if (i%(1<<k) == 0){
					if (((i + (1<<(k-1))) < P) && ((1<<k) < P)){
                    	players[i][k].role = WINNER;
						players[i][k].opponent = i+(1<<(k-1));
					} else if ((i + (1<<(k-1))) >= P){
						players[i][k].role = BYE;
						players[i][k].opponent = -1;
					}
				}
				else if ((i%(1<<k)) == (1<<(k-1))){
					players[i][k].role = LOSER;
					players[i][k].opponent = i-(1<<(k-1));
				}
			}
			printf("Player role %d\n", players[i][k].role );
			printf(" opponent %d\n", players[i][k].opponent );
			printf("flag %d\n", players[i][k].flag );
		}
	}
}

void tournament_barrier(int *pid_sense)
{
	printf("In barrier by thread %d\n", omp_get_thread_num());
	int i = 0;
	int flag = 0;
	int vpid = omp_get_thread_num();
	//Arrival
	while (flag)
	{
		i += 1;
		printf("Processor %d reached barrier in round %d\n", vpid, i);
		switch (players[vpid][i].role)
		{
			case WINNER:
				while (players[vpid][i].flag == *pid_sense);
			case LOSER:
				players[players[vpid][i].opponent][i].flag = *pid_sense;
				break;
			case BYE: 
				break;
			case CHAMPION:
				while (players[vpid][i].flag == *pid_sense);
				players[players[vpid][i].opponent][i].flag = *pid_sense;
				break;
			// case DROPOUT:
			// 	break;
		}
	}

	//Wakeup
	flag = 1;
	while (flag)
	{
		i -= 1;
		switch (players[vpid][i].role)
		{
			case WINNER:
				players[players[vpid][i].opponent][i].flag = *pid_sense;
			case LOSER:
				break;
			case BYE:
				break;
			case CHAMPION:
				break;
			case DROPOUT:
				flag = 0;
				break;
		}
	}
	*pid_sense = 1-*pid_sense;	
}

int main(int argc, char* argv[])
{
	if (argc == 3){
        if (sscanf (argv[1], "%d", &P)!=1)
        	printf ("P - not an integer\n");
        if (sscanf (argv[2], "%d", &N)!=1)
        	printf ("N - not an integer\n");
    }
    else{
    	//Number of processors
		P = 8;
		//Number of loops
		N = 4;
    }
    struct timeval tv1, tv2;
    double total_time
	int pid_sense = 1;
	printf("In main\n");
	barrier_init();
	#pragma omp parallel num_threads(P) shared (players, tv1, tv2) firstprivate(pid_sense, N)
	{
		int i;
		gettimeofday(&tv1, NULL);
		for(i=0;i<N;i++)
		{
			printf("Before first barrier\n");
			tournament_barrier(&pid_sense);
			printf("Before second barrier\n");
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
			tournament_barrier(&pid_sense);
		}	
		gettimeofday(&tv2, NULL);
	}
	total_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec)*1000000;
    printf("\nSUMMARY:\nTotal run-time for %d "
            "loops with 5 barriers per loop: %fs\n"
            "The average time per barrier: %fus\n",
            N, total_time/1000000, (double)(total_time/(N*5)));
	free(players);
	return 0;
}
