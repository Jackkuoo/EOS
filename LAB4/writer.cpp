#include <cstdio>       // fprintf(), perror()
#include <cstdlib>      // exit()
#include <cstring>      // memset()
#include <csignal>      // signal()
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()
#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons()
#include <string>

int fd;
int sec;
char name[8] = {};

void sigint_handler(int signo) {
    close(fd);
}

int main(int argc, char *argv[]) {
    
    signal(SIGINT, sigint_handler);
    // Open driver
    if((fd = open(argv[2], O_RDWR)) < 0) {
        perror(argv[2]);
        exit(EXIT_FAILURE);
    }
    else{
	memset(name, '\0', sizeof(name));
	strcpy(name, argv[1]);
	printf("write1 %s \n",name);
	int ret;
	char buf[16]={};
	for(int i = 0; i < 8; i++) {
		sprintf(buf, "%c", name[i]);
    		if((write(fd, &buf, sizeof(buf))) == -1) {
	    		perror("write()");
        		exit(EXIT_FAILURE);
		}
	   	printf("write2 %c\n",name[i]);
		sleep(1);
	}
	close(fd);
    }
    return 0;
}
