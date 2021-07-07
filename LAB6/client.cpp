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
#define errexit(format,arg...) exit(printf(format,##arg))
typedef struct sockaddr *sockaddrp;

int connect_init(string port, string ip){

    struct sockaddr_in addr = {AF_INET};
    addr.sin_port = htons(atoi(port.c_str()));
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t addr_len = sizeof(addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        errexit("Can't create socket: %s\n" , strerror(errno));

    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
    	errexit("Can't connect to socket: %s\n" , strerror(errno));
    }

    return fd;
}

int main(int argc, char *argv[]) {
    
    int connfd,n;
    char snd[1024];
    /* ./client 127.0.0.1 $PORT withdraw 1 500 */
    connfd = connect_init(argv[2],argv[1]);
    sprintf(snd, "%s %d %d",argv[3], atoi(argv[4]), atoi(argv[5]));
    if((n = write(connfd, snd, sizeof(snd))) == -1){
    	errexit("Error: write()\n");
    }
    close(connfd);
    return 0;
}