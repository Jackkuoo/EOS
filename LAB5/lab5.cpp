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
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define errexit(format,arg...) exit(printf(format,##arg))
#define MAX_SOCKET_CONNECTION 80
typedef struct sockaddr *sockaddrp;
int sockfd;


void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler(int signum) {
    close(sockfd);
    exit(0);
}

void childprocess(int fd) {
    cout << "Train ID: " << getpid() << endl;
    dup2(fd, STDOUT_FILENO); 
    execlp("sl", "-l", NULL); 
}


int socket_init(string port, string ip){

    struct sockaddr_in addr = {AF_INET};
    addr.sin_port = htons(atoi(port.c_str())); /* host to network short*/
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    socklen_t addr_len = sizeof(addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0); /* allocate the socket */
    if(sockfd < 0)
        errexit("Can't create socket: %s\n" , strerror(errno));

    if(bind(sockfd, (sockaddrp)&addr, addr_len) < 0) /* bind the socket */ 
        errexit("Can't bind to port %s: %s\n", port.c_str(), strerror(errno));

    if(listen(sockfd, MAX_SOCKET_CONNECTION)<0) /*set the maximum number of waiting connection*/
        errexit("Can't listen on port %s: %s\n", port.c_str(), strerror(errno));

    return sockfd;
}



int main(int argc, char *argv[]) {

    pid_t childpid; /* variable to store the child ’s pid */
    int conn_fd_list[MAX_SOCKET_CONNECTION] = {0};  
    struct sockaddr_in addr_cln[MAX_SOCKET_CONNECTION];
    socklen_t sLen = sizeof(addr_cln[0]);
    int num_conn = 0, connfd;   

    int sockfd = socket_init(argv[1], "127.0.0.1");

    signal(SIGCHLD, zombie_handler);
    signal(SIGINT, handler);
    
    while(1){
        connfd = accept(sockfd, (sockaddrp)&addr_cln[num_conn], &sLen); /* waiting for connection */
        if(connfd != -1) {
            conn_fd_list[num_conn] = connfd;

            childpid = fork();
            if(childpid >= 0){
                if(childpid == 0) {
                    childprocess(conn_fd_list[num_conn]);
                }else{
                    num_conn++;
                }
            }else { 
                perror("fork");
                exit(0);
            }
        }
    }
    return 0;
}
