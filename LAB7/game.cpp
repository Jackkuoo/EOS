#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

typedef struct {
    int guess;
    char result[8];
}data;

key_t key;
int num;
int shmid;
void *shm;
data *gamedata;

void USR_handler(int signo, siginfo_t *info, void *context)
{
    //printf("Process (%d) sent SIGUSR1\n", info->si_pid);
    if(gamedata->guess < num) {
        sprintf(gamedata->result, "bigger");
        kill(info->si_pid, SIGUSR1);
    } else if(gamedata->guess > num){
        sprintf(gamedata->result, "smaller");
        kill(info->si_pid, SIGUSR1);
    } else {
        sprintf(gamedata->result, "bingo");
        kill(info->si_pid, SIGINT);
    }
    cout << "Guess: " << gamedata->guess << ", " << gamedata->result << endl;
}

void INT_handler(int signum) {
    shmdt(shm); 
    shmctl(shmid, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char *argv[]) {
    key = atoi(argv[1]);
    num = atoi(argv[2]);

    struct sigaction my_action;
    memset(&my_action, 0, sizeof(struct sigaction));
    my_action.sa_flags = SA_SIGINFO;
    my_action.sa_sigaction = USR_handler;
    sigaction(SIGUSR1, &my_action, NULL);

    signal(SIGINT, INT_handler);

    if ((shmid = shmget(key, sizeof(data), 0666|IPC_CREAT)) < 0){
        perror("shmget");
        exit(1);
    }
    if ((shm = shmat(shmid, (void *)0, 0)) == (void *)-1){
        perror("shmat");
        exit(1);
    }

    gamedata = (data *)shm;
    
    cout << "Game PID: " << getpid() << endl;

    struct timespec req;
    memset(&req, 0, sizeof(struct timespec));
    req.tv_sec = 60;
    req.tv_nsec = 0;
    int retval;
    do {
        retval = nanosleep(&req, &req);
    }while(retval);
    
    shmdt(shm); 
    
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}