#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <csignal> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;
#define SEM_MODE 0666
#define errexit(format,arg...) exit(printf(format,##arg))
#define MAX_SOCKET_CONNECTION 80
typedef struct sockaddr *sockaddrp;
int sockfd;
int money=0; 
int s;

void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler(int signum) {
    if(semctl(s, 0, IPC_RMID, 0)<0){
        errexit("unable to remove samophore: %s\n", strerror(errno));
    }
    printf("Semaphore has been remove\n");
    close(sockfd);
    pthread_exit(NULL);
    exit(0);
}
/* P() return 0 if OK, -1 if problem*/
int P(){
    struct sembuf sop;
    sop.sem_num = 0;  /* access 1st semaphore*/
    sop.sem_op = -1;  /* wait */
    sop.sem_flg = 0;  /* no special optioins needed */
    if(semop(s, &sop, 1) < 0){
        errexit("semop failed: %s\n", strerror(errno));
        return -1;
    }else{
        return 0;
    }
}
/* V() return 0 if OK, -1 if problem*/
int V(){
    struct sembuf sop;
    sop.sem_num = 0;  /* access 1st semaphore*/
    sop.sem_op = 1;  /* signal */
    sop.sem_flg = 0;  /* no special optioins needed */
    if(semop(s, &sop, 1) < 0){
        errexit("semop failed: %s\n", strerror(errno));
        return -1;
    }else{
        return 0;
    }
}

void *atm(void *connfd)
{
    int fd;
    char rcv[1024];
    fd = *((int*)(&connfd));
    int n;
    if((n = read(fd, rcv, sizeof(rcv))) == -1){
        errexit("Error read: %s\n", strerror(errno));
    }

    int count, num;
    char depo_with[10] = {0};
    sscanf(rcv, "%s %d %d", depo_with, &num, &count);
    printf("do %s %d %d\n", depo_with, num, count);
    for(int i=0;i<count;i++){
        P();
        if(strncmp(depo_with,"deposit",7)==0){
            money = money + num;
        }else{
            money = money - num;
        }
        printf("After %s %d: %d\n", depo_with, num, money);
        V();
    }
    return 0;
}

int server_init(string port, string ip){

    struct sockaddr_in sock = {AF_INET};
    sock.sin_port = htons(atoi(port.c_str()));
    sock.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t addr_len = sizeof(sock);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        errexit("Can't create socket: %s\n" , strerror(errno));

    if(bind(fd, (sockaddrp)&sock, addr_len) < 0) 
        errexit("Can't bind to port %s: %s\n", port.c_str(), strerror(errno));

    if(listen(fd, MAX_SOCKET_CONNECTION)<0)
        errexit("Can't listen on port %s: %s\n", port.c_str(), strerror(errno));

    return fd;
}

int main(int argc, char *argv[]) {
    pthread_t threads[80];
    int conn_fd_list[MAX_SOCKET_CONNECTION] = {0};  
    struct sockaddr_in addr_cln[MAX_SOCKET_CONNECTION];
    socklen_t sLen = sizeof(addr_cln[0]);
    int num_conn = 0, connfd;   

    int sockfd = server_init(argv[1], "127.0.0.1");

    signal(SIGCHLD, zombie_handler);
    signal(SIGINT, handler);
    
    if(s = semget(1234567, 1, IPC_CREAT|SEM_MODE)){
        printf("creation of semaphore failed: %s\n", strerror(errno));
    }

    if(semctl(s, 0, SETVAL, 1) < 0){
        errexit("initializaiton of semaphore failed: %s\n", strerror(errno));
    }

    while(1){
        connfd = accept(sockfd, (sockaddrp)&addr_cln[num_conn], &sLen);
        if(connfd != -1) {
            conn_fd_list[num_conn] = connfd;

            if(pthread_create(&threads[num_conn], NULL, atm, (void *)connfd)){
                errexit("thread create failed: %s\n", strerror(errno));
            }
        }
    }
    return 0;
}