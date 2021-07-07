#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>

using namespace std;

typedef struct {
    int guess;
    char result[8];
}data;

key_t key;
int game_pid;
int high_num = 101, low_num = 0;
int shmid;
void *shm;
data *gamedata;

void USR_handler(int signo, siginfo_t *info, void *context)
{
    string result = string(gamedata->result);
    if(result == "smaller") {
        high_num = gamedata->guess;
    }else if (result == "bigger") {
        low_num = gamedata->guess;
    }else{
        kill(getpid(), SIGINT);
    }
}

void VTALRM_handler(int signum) {
    gamedata->guess = (high_num + low_num) / 2;
    cout << "Guess: " << gamedata->guess << endl;
    kill(game_pid, SIGUSR1);
}

int main(int argc, char *argv[]) {
    key = atoi(argv[1]);
    high_num = atoi(argv[2])+1;
    game_pid = atoi(argv[3]);

    if ((shmid = shmget(key, sizeof(data), 0666|IPC_CREAT)) < 0){
        perror("shmget");
        exit(1);
    }
    if ((shm = shmat(shmid, (void *)0, 0)) == (void *)-1){
        perror("shmat");
        exit(1);
    }

    gamedata = (data *)shm;
    

    struct sigaction my_action;
    memset(&my_action, 0, sizeof(struct sigaction));
    my_action.sa_flags = SA_SIGINFO;
    my_action.sa_sigaction = USR_handler;
    sigaction(SIGUSR1, &my_action, NULL);

    struct sigaction timer_action;
    struct itimerval timer;
    memset(&timer_action, 0, sizeof(timer_action));
    timer_action.sa_handler = &VTALRM_handler;
    sigaction(SIGVTALRM, &timer_action, NULL);
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    
    while(1);

    return 0;
}