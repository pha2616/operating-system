#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#define QUEUE_SIZE 5
#define PROCESS_SIZE 25

struct Process{
	int pID, pri, bt, weight, delta_exec, vruntime;
};

struct btSumEle{
	int idx, btSum;
};

void *MultiQueue(void *RQ);
void *SJF(void *RQ);
void *Priority(void *RQ);
void *RR(void *RQ);
void *CFS(void *RQ);
void *FRFS(void *RQ);

sem_t mutex;
int thr_id;
//save each queue's bt sum
struct btSumEle btSum[QUEUE_SIZE];

int main(){
	struct Process* readyQueue[QUEUE_SIZE];
	struct Process pr;
	int classNo, pID, pri, bt, thr_id, status; int idx[5] = { 0, 0, 0, 0, 0};
	pthread_t p_thread[QUEUE_SIZE];

	for(int i = 0; i< QUEUE_SIZE; ++i){
		readyQueue[i] = (struct Process*)malloc(sizeof(struct Process)*(PROCESS_SIZE + 1));
	}

	for(int i = 0; i < PROCESS_SIZE; ++i){
		scanf("%d %d %d %d", &classNo, &pID, &pri, &bt);
		getchar();
		pr.pID = pID;
		pr.pri = pri;
		pr.bt = bt;
		readyQueue[classNo][idx[classNo]++] = pr;
		btSum[classNo].btSum+=pr.bt;
		btSum[classNo].idx = classNo;
	}

	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
        thr_id = pthread_create(&pid, &attr, MultiQueue, (void*)readyQueue);
	if(thr_id < 0){
		perror("error \n");
		exit(0);
	}
	pthread_join(pid, NULL);
}

void* MultiQueue(void *RQ){
	sem_init(&mutex, 0, 1);
	struct Process** Q = (struct Process**)RQ;
	pthread_t p_thread[QUEUE_SIZE];
	pthread_attr_t attr[QUEUE_SIZE];
	for(int i = 0;i < QUEUE_SIZE;++i){
		pthread_attr_init(&attr[i]);
	}
	//insert sorting
	int i,j;
	for(i = 1; i<QUEUE_SIZE; ++i){
		int key = btSum[i].btSum, keyIdx = btSum[i].idx;
		for(j = i - 1; j >= 0; --j){
			if(btSum[j].btSum > key){
				btSum[j + 1].btSum = btSum[j].btSum;
				btSum[j + 1].idx = btSum[j].idx;
			}
			else break;
		}
		btSum[j + 1].btSum = key, btSum[j + 1].idx = keyIdx;
	}

	for(int i = 0;i<QUEUE_SIZE;++i){
		if(btSum[i].idx == 0){
			pthread_create(&p_thread[btSum[i].idx],&attr[btSum[i].idx],SJF,Q);
			pthread_join(p_thread[btSum[i].idx],NULL);
		}
		else if(btSum[i].idx == 1){
			pthread_create(&p_thread[btSum[i].idx],&attr[btSum[i].idx],Priority,Q);
			pthread_join(p_thread[btSum[i].idx],NULL);
		}
		else if(btSum[i].idx == 2){
			pthread_create(&p_thread[btSum[i].idx],&attr[btSum[i].idx],RR,Q);
			pthread_join(p_thread[btSum[i].idx],NULL);
		}
		else if(btSum[i].idx == 3){
			pthread_create(&p_thread[btSum[i].idx],&attr[btSum[i].idx],CFS,Q);
			pthread_join(p_thread[btSum[i].idx],NULL);
		}
		else{
			pthread_create(&p_thread[btSum[i].idx],&attr[btSum[i].idx],FRFS,Q);
			pthread_join(p_thread[btSum[i].idx],NULL);
		}
	}
}

void *SJF(void *RQ){
	sem_wait(&mutex);
	struct Process** rq = (struct Process**)RQ;
	
	int tmpBT, tmpPID;
	//bubble sort
	for(int i = 0; i < QUEUE_SIZE; ++i){
		for(int j = 0; j < QUEUE_SIZE - i - 1; ++j){
			//pID only larger than 0
			if(rq[0][j].pID <= 0 || rq[0][j + 1].pID <=0) continue;
			if(rq[0][j].bt > rq[0][j + 1].bt){
				tmpBT = rq[0][j].bt, tmpPID = rq[0][j].pID;
				rq[0][j].bt = rq[0][j + 1].bt, rq[0][j].pID = rq[0][j + 1].pID;
				rq[0][j + 1].bt = tmpBT, rq[0][j + 1].pID = tmpPID;
			}
		}
	}

	for(int i = 0; i < QUEUE_SIZE; ++i){
		if(rq[0][i].pID > 0) {
			while(rq[0][i].bt--){
		         	printf("%d ", rq[0][i].pID);
			}
		}
		printf("\n");
	}
	sem_post(&mutex);

	pthread_exit(NULL);
}

void *Priority(void *RQ){
	sem_wait(&mutex);
	struct Process** rq = (struct Process**)RQ;
	
	int tmpBT, tmpPID, tmpPRI;
	for(int i = 0; i < QUEUE_SIZE; ++i){
		for(int j = 0; j < QUEUE_SIZE - i - 1; ++j){
			if(rq[1][j].pID <= 0 || rq[1][j + 1].pID <= 0 ) continue;
			if(rq[1][j].pri > rq[1][j + 1].pri){
				tmpBT = rq[1][j].bt, tmpPID = rq[1][j].pID, tmpPRI = rq[1][j].pri;
				rq[1][j].bt = rq[1][j + 1].bt, rq[1][j].pID = rq[1][j + 1].pID, rq[1][j].pri = rq[1][j+1].pri;
				rq[1][j+1].bt = tmpBT, rq[1][j+1].pID = tmpPID, rq[1][j+1].pri = tmpPRI;
			}
		}
	}

	for(int i = 0; i < QUEUE_SIZE; ++i){
		if(rq[1][i].pID > 0){
			while(rq[1][i].bt--){
				printf("%d ",rq[1][i].pID);
			}
			printf("\n");
		}
	}
	sem_post(&mutex);
	//
	pthread_exit(NULL);
}

void *RR(void *RQ){
	sem_wait(&mutex);
	struct Process** rq=(struct Process**)RQ;
	//circular queue
	struct Process q[QUEUE_SIZE];
	//queue index
	int f = 0, r = 0;
	//time quantum
	int tq = 3, btSum = 0;
	//insert each process to queue
	while(r != QUEUE_SIZE){
		q[r] = rq[2][r];
		if(q[r].pID > 0) btSum += q[r].bt;
		r++;
	}

	//Round-Robin
	while(btSum){
		if(q[f].bt > tq){
			int cnt = tq;
			while(cnt--){
				printf("%d ",q[f].pID);
				q[f].bt--,btSum--;
			}
			printf("\n");
		}
		else if(q[f].bt > 0){
			while(q[f].bt != 0){
				printf("%d ", q[f].pID);
				q[f].bt--,btSum--;
			}
			printf("\n");
		}
		f = (++f) % QUEUE_SIZE;
	}
	sem_post(&mutex);
	pthread_exit(NULL);
}

struct Min_Vruntime{
	int idx, min_vruntime;
};

void *CFS(void* RQ){
	sem_wait(&mutex);
	struct Process** rq = (struct Process**)RQ;
	int NICE_0_LOAD = 1024, totalTime = 62, exec = 2, execCnt = totalTime/exec;
	int load_weight[QUEUE_SIZE + 1] = {0, 1, 2, 4, 8, 16}; //each process weight
	struct Min_Vruntime mv;
	mv.min_vruntime = 1000000000 , mv.idx = 0;
	//initialization
	for(int i=0; i < QUEUE_SIZE; ++i){
		rq[3][i].delta_exec  = 0;
		//convert priority(from input) to weight(from load_weighyt array)
		rq[3][i].weight = load_weight[rq[3][i].pri];
		//exec randomly  if vruntime is 0
		rq[3][i].vruntime += NICE_0_LOAD / rq[3][i].weight * exec;
		int cnt = exec;
		execCnt--;
		while(rq[3][i].bt && cnt--){       
			rq[3][i].bt--;
			printf("%d ", rq[3][i].pID);
		}
		printf("\n");
		
		rq[3][i].delta_exec += exec;
		if(mv.min_vruntime > rq[3][i].vruntime){
			mv.min_vruntime = rq[3][i].vruntime;
			mv.idx = i;
		}
	}

	while(execCnt--){
		if(!rq[3][mv.idx].bt) continue;
		rq[3][mv.idx].delta_exec += exec;
		rq[3][mv.idx].vruntime += NICE_0_LOAD / rq[3][mv.idx].weight * rq[3][mv.idx].delta_exec;
		mv.min_vruntime = rq[3][mv.idx].vruntime;
		
		int cnt = exec;
		while(rq[3][mv.idx].bt && cnt--){
		       	rq[3][mv.idx].bt--;
			printf("%d ", rq[3][mv.idx].pID);
		}
		printf("\n");
		
		//find min_vruntime
		for(int i = 0; i < QUEUE_SIZE; ++i){
			if(i == mv.idx || rq[3][i].bt <= 0) continue;
			if(rq[3][i].vruntime <= mv.min_vruntime){
				mv.min_vruntime = rq[3][i].vruntime;
				mv.idx = i;
			}
		}
	}

	while(rq[3][mv.idx].bt--){
		printf("%d ",rq[3][mv.idx].pID);
	};
	sem_post(&mutex);
	pthread_exit(NULL);
}

void *FRFS(void* RQ){
	sem_wait(&mutex);
	struct Process** rq = (struct Process**)RQ;

	for(int i = 0; i < QUEUE_SIZE; ++i){
		while(rq[4][i].bt--){
			printf("%d ",rq[4][i].pID);
		}
		printf("\n");
	}
	sem_post(&mutex);
	pthread_exit(NULL);
}
