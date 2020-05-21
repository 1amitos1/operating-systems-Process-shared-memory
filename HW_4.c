#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#define _BSD_SOURCE
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include<sys/sem.h>
#include<sys/wait.h>
#include<errno.h>
#include<sys/ipc.h>
#include <termios.h>
#include <math.h>
#include <unistd.h>

#define BSIZE 100 // Buffer size
#define SEMPERM 0600
#define TRUE 1
#define FALSE 0


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

struct timeval t;
time_t t1, t2,t3,t4;
int consumer,producer;
int TIME_1,TIME_2,TIME_3,NUMBER_OF_STATIONS;
int shmid1, shmid2, shmid3, shmid4,semid,semid2,shmid_conter,smid_waiting_time,shmid_flag2;
key_t k1 = 5491, k2 = 5812, k3 = 423127, k4 = 3213,k5=258774, k6=32587,k7=1258741,semkey=12528,semkey2=1745;
bool* SHM1;
int* SHM2;
int* SHM3;

int*SHM_CONTER;
int*SHM_WAITING_TIME;
bool *SHM_FLAG_2;
static int num=0;

typedef union _semun
{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
}semun;


int nextTime1(int rateParameter)
{
//	printf("for simulation time 30 sec avg\n");
	time_t t;
	float res=0;
	srand((unsigned)time(&t));
	float res2=0;

	res= -log(1.0f -(float)rand() / (float)(RAND_MAX)) /rateParameter;
	res2=(res*1000);
	if(res2>40)
	{
		res2=30;
	}
	if(res2<15)
	{
		res2=20;
	}
	return (int)res2;
}
int nextTime2(float rateParameter)
{
	//printf("for Wash time 3 sec avg\n");
	time_t t;
	float res=0;
	float res2=0;

	srand((unsigned)time(&t));

	res= -log(1.0f -(float)rand() / (float)(RAND_MAX)) /rateParameter;
	res2=(res*10+1);
	if(res2>6)
	{
		res2=4;
	}
	return (int)res2;
}
int nextTime3(float rateParameter)
{
  //printf("Time for the arrival of vehicles 1.5 sec avg\n");
  time_t t;
  float res=0;
  srand((unsigned)time(&t));
  float res2=0;
  res= -log(1.0f -(float)rand() / (float)(RAND_MAX)) /rateParameter;
  res2=(res*10+1);
  if(res2>=3)
  {
    res2=2;
  }
  return (int)res2;
}

void sig_handle(int sig)
{
  printf(BLU"\n[SIGUSR1]THE CLOCK RAN OUT!!\n"RESET);
  int* buf = SHM3;
  int index;
  int last=0;
  int status;
  int last_index=0;
  while(buf[last_index]!=0)
  {
    kill(buf[last_index],SIGCONT);
    last_index++;
  }
  waitpid(buf[last_index-1],&status,WUNTRACED );
  waitpid(buf[last_index-1],&status,WUNTRACED);
  int* state = (int*)shmat(shmid4, NULL, 0);
  *state = 0;
  exit(EXIT_SUCCESS);
}
void sig_handle2(int sig)
{
//  printf("\n[Producer]SIGINT  pid=%d Consumer=%d \n",getpid(),consumer);

  kill(getpid(),SIGUSR1);

}
void sig_handle3(int sig)
{
  //printf("[CAR]SIGINT SIGNAL A CAR pid=%d Consumer=%d \n",getpid(),consumer);
  //kill(getpid(),SIGTERM);
  //exit(0);
}

void sig_main_handler(int sig)
{
  //printf("MAIN-sig SIGINT producer=%d consumer=%d\n",producer,consumer);
  kill(producer,SIGTERM);
  kill(consumer,SIGUSR1);
}

int p(int semid)
{
  struct sembuf p_buf;
  p_buf.sem_num = 0;
  p_buf.sem_op = -1;
  p_buf.sem_flg = SEM_UNDO;

	//int semop(int semid, struct sembuf *sops, size_t nsops);
  int temp=semop(semid, &p_buf, 1);
  if( temp == -1 )
  {
    perror("Error operation p(semid)");
    exit(1);
  }
  return 0;
}

int v(int semid)
{
  struct sembuf v_buf;
  v_buf.sem_num = 0;
  v_buf.sem_op = 1;
  v_buf.sem_flg = SEM_UNDO;

	//int semop(int semid, struct sembuf *sops, size_t nsops);
  if( semop(semid, &v_buf, 1) == -1 )
  {
    perror("Error operation v(semid)");
    exit(1);
  }
  return 0;
}

int initsem(key_t semkey)
{
  int status = 0;

	//int semget(key_t key, int nsems, int semflg);
  if( ( semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL ) ) == -1 )
  {
    if( errno == EEXIST )
    {
      semid = semget( semkey, 1, SEMPERM );
    }
  }
  else
  {
    union _semun arg;
    arg.val = NUMBER_OF_STATIONS;

    // int semctl(int semid, int semnum, int cmd, union semun arg);
    status = semctl(semid, 0, SETVAL, arg);
  }

  if( semid == -1 || status == -1 )
  {
    perror("Error initsem");
    exit(-1);
  }
  return (semid);
}

int initsem_2(key_t semkey2)
{
  int status = 0;

	//int semget(key_t key, int nsems, int semflg);
  if( ( semid2 = semget(semkey2, 1, SEMPERM | IPC_CREAT | IPC_EXCL ) ) == -1 )
  {
    if( errno == EEXIST )
    {
      semid2 = semget( semkey2, 1, SEMPERM );
    }
  }
  else
  {
    union _semun arg;
    arg.val = 1;

    // int semctl(int semid, int semnum, int cmd, union semun arg);
    status = semctl(semid2, 0, SETVAL, arg);
  }

  if( semid2 == -1 || status == -1 )
  {
    perror("Error initsem");
    exit(-1);
  }
  return (semid2);
}

void handlesem(key_t semkey ,key_t semkey2,time_t t3 )
{
    signal(SIGINT,sig_handle3);
    signal(SIGQUIT,sig_handle3);
    signal(SIGTSTP,sig_handle3);

    int semid;
    pid_t pid = getpid();

    if( ( semid = initsem(semkey) ) < 0 )
    {
      exit(1);
    }
    if( (semid2 = initsem_2(semkey2) ) < 0 )
    {
      exit(1);
    }
    p(semid);
    p(semid2);

    shmid_conter=shmget(k5, sizeof(int) * 1, IPC_CREAT | 0660);//FOR COuNTING CARS
    SHM_CONTER = (int*)shmat(shmid_conter, NULL, 0);
    int* count=SHM_CONTER;
    *count= *(count)+1;


    SHM_WAITING_TIME = (int*)shmat(smid_waiting_time, NULL, 0);
    *SHM_WAITING_TIME=0;
    int* waiting_time=SHM_WAITING_TIME;

    gettimeofday(&t, NULL);
    t2 = t.tv_sec;
    *waiting_time= *(waiting_time)+(t2-t1);


    printf(YEL"\n[B][B]CAR %d goes into the wash,Time passed:%ld,Arrival_Time=%ld\n\n",pid,TIME_1-(t2-t1),t3);
    v(semid2);

    sleep(TIME_2);

    p(semid2);
    gettimeofday(&t, NULL);
    t2 = t.tv_sec;


    t2 = t.tv_sec;
    printf(GRN"\n[C][C]CAR %d finished the wash,Time passed:%ld\n\n",pid,TIME_1-(t2-t1));

    v(semid2);
    v(semid);


    exit(EXIT_SUCCESS);

}

int myrand(int n) // Returns a random number between 1 and n
{
    time_t t;
    srand((unsigned)time(&t));
    return (rand() % n + 1);
}


int Creating_cars()
{
  pid_t p;
  int status;
  long res= 0;

  p=fork();

  if(p==0)
  {
    signal(SIGINT,sig_handle3);
    signal(SIGQUIT,sig_handle3);
    signal(SIGTSTP,sig_handle3);

    gettimeofday(&t, NULL);
    t2 = t.tv_sec;

    printf(RED"\n[A][A]The CAR %d arrived at time  %ld\n\n"  ,getpid(),t2-t1);
    kill(getpid(),SIGSTOP);
    t3=t2-t1;
    SHM_WAITING_TIME = (int*)shmat(smid_waiting_time, NULL, 0);
    *SHM_WAITING_TIME=0;
    int* waiting_time=SHM_WAITING_TIME;
    *waiting_time= *(waiting_time)-t3;

    handlesem(semkey,semkey2,t3);
  }
  else
  {

    signal(SIGINT,sig_handle2);
    signal(SIGQUIT,sig_handle2);
    signal(SIGTSTP,sig_handle2);
    waitpid(p,&status,WUNTRACED);
    return p;
  }
}

int main()
{

    //int TIME_1,TIME_2,TIME_3,NUMBER_OF_STATIONS;
    printf(WHT"ENTR Number of stations\n"RESET);
    scanf("%d",&NUMBER_OF_STATIONS);
    TIME_1=nextTime1(30);
    TIME_2=nextTime2(3);
    TIME_3=nextTime3(1.5);


    shmid1 = shmget(k1, sizeof(bool) * 2, IPC_CREAT | 0660); // flag
    shmid2 = shmget(k2, sizeof(int) * 1, IPC_CREAT | 0660); // turn
    shmid3 = shmget(k3, sizeof(int) * BSIZE, IPC_CREAT | 0660); // buffer
    shmid4 = shmget(k4, sizeof(int) * 1, IPC_CREAT | 0660); // time stamp
    shmid_conter=shmget(k5, sizeof(int) * 1, IPC_CREAT | 0660);//car count
    smid_waiting_time=shmget(k6, sizeof(int) * 1, IPC_CREAT | 0660);


    //Initialize car count
    SHM_CONTER = (int*)shmat(shmid_conter, NULL, 0);
    *SHM_CONTER=0;
    int* count=SHM_CONTER;

    SHM_WAITING_TIME = (int*)shmat(smid_waiting_time, NULL, 0);
    *SHM_WAITING_TIME=0;
    int* waiting_time=SHM_WAITING_TIME;



    if (shmid1 < 0 || shmid2 < 0 || shmid3 < 0 || shmid4 < 0) {
        perror("Main shmget error: ");
        exit(1);
    }

    SHM3 = (int*)shmat(shmid3, NULL, 0);
    int ix = 0;

    while (ix < BSIZE) // Initializing buffer
        SHM3[ix++] = 0;


    gettimeofday(&t, NULL);
    t1 = t.tv_sec;

    int* state = (int*)shmat(shmid4, NULL, 0);
    *state = 1;
    int wait_time;

    int i = 0; // Consumer
    int j = 1; // Producer


    producer=fork();
    if (producer == 0) // Producer code
    {
        signal(SIGINT,sig_handle2);
        signal(SIGQUIT,sig_handle2);
        signal(SIGTSTP,sig_handle2);
        SHM1 = (bool*)shmat(shmid1, NULL, 0);
        SHM2 = (int*)shmat(shmid2, NULL, 0);
        SHM3 = (int*)shmat(shmid3, NULL, 0);
        if (SHM1 == (bool*)-1 || SHM2 == (int*)-1 || SHM3 == (int*)-1) {
            perror("Producer shmat error: ");
            exit(1);
        }

        bool* flag = SHM1;
        int* turn = SHM2;
        int* buf = SHM3;
        int index = 0;

        while (*state == 1) {
            flag[j] = true;
            *turn = i;
            while (flag[i] == true && *turn == i);
            // Critical Section Begin
            index = 0;
            while (index < BSIZE)
             {
                if (buf[index] == 0) {
                    int tempo = Creating_cars();
                    buf[index] = tempo;
                    break;
                }
                index++;
            }
            if (index == BSIZE)
                printf("Buffer is full, nothing can be produced!!!\n");
            flag[j] = false;
            if (*state == 0)
                break;
            sleep(TIME_3);
        }
        exit(0);

    }

    consumer=fork();
    if (consumer == 0) // Consumer code
    {
        signal(SIGUSR1,sig_handle);
        signal(SIGINT,sig_handle2);
        signal(SIGQUIT,sig_handle2);
        signal(SIGTSTP,sig_handle2);
        SHM1 = (bool*)shmat(shmid1, NULL, 0);
        SHM2 = (int*)shmat(shmid2, NULL, 0);
        SHM3 = (int*)shmat(shmid3, NULL, 0);


        shmid_conter=shmget(k5, sizeof(int) * 1, IPC_CREAT | 0660);//FOR COuNTING CARS
        SHM_CONTER = (int*)shmat(shmid_conter, NULL, 0);
        int* count=SHM_CONTER;


        if (SHM1 == (bool*)-1 || SHM2 == (int*)-1 || SHM3 == (int*)-1) {
            perror("Consumer shmat error:");
            exit(1);
        }

        bool* flag = SHM1;
        int* turn = SHM2;
        int* buf = SHM3;
        int index = 0;
        flag[i] = false;
        int status;
        sleep(2);
        while (*state == 1)
         {
            flag[i] = true;
            int first;
            *turn = j;
            while (flag[j] == true && *turn == j);
            if (buf[0] != 0)
             {
                first=buf[0];
                buf[0] = 0;
                index = 1;
                kill(first,SIGCONT);
                int index_2=5;
                while (index < BSIZE) // Shifting remaining jobs forward
                {
                    if(buf[index-1]!=0 && index_2>0)
                    {
                        kill(buf[index-1],SIGCONT);
                        index_2--;
                    }
                    buf[index - 1] = buf[index];
                    index++;
                }
                buf[index - 1] = 0;
             }
             else
             {
               printf("Buffer is empty, nothing can be consumed!!!\n");
               sleep(2);
             }
            flag[i] = false;
            if (*state == 0)
                break;
            wait_time = myrand(5);
            sleep(wait_time);
        }
        exit(0);
    }

    else
    {
      while (1)
       {
         signal(SIGINT,sig_main_handler);
         signal(SIGQUIT,sig_main_handler);
         signal(SIGTSTP,sig_main_handler);

          gettimeofday(&t, NULL);
          t2 = t.tv_sec;
          if (t2 - t1 > TIME_1) // Program will exit after RT seconds
          {
              kill(producer,SIGTERM);
              kill(consumer,SIGUSR1);
              //kill(consumer,SIGSTOP);

              //*state = 0;
              break;
          }
      }
    }

    signal(SIGINT,sig_main_handler);
    signal(SIGQUIT,sig_main_handler);
    signal(SIGTSTP,sig_main_handler);
    int status=0;
    sleep(10);
    waitpid(consumer,&status,WUNTRACED);

    shmid_conter=shmget(k5, sizeof(int) * 1, IPC_CREAT | 0660);//FOR COuNTING CARS
    SHM_CONTER = (int*)shmat(shmid_conter, NULL, 0);
    SHM_WAITING_TIME = (int*)shmat(smid_waiting_time, NULL, 0);

    printf(CYN"[+][+]Summary\n");
    printf(CYN"NUMBER OF TOTAL CARS WASHED =%d\n",*count);
    printf(CYN"\nWAITING_TIME=%d\n",*waiting_time);
    printf(CYN"\nAVG__WAITING_TIME=%f\n",(float)(*waiting_time)/(*count));
    semctl(semkey, 0, IPC_RMID, NULL);
    return 0;
}
