#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h> // socket
#include <sys/wait.h>   // wait pid
#include <arpa/inet.h> // socket addr convert
#include <unistd.h> // delay
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <mutex>
#include <thread>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

using namespace std;
#define SEM_MODE 0666
#define errexit(format,arg...) exit(printf(format,##arg))
#define MAX_SOCKET_CONNECTION 1000

typedef struct sockaddr *sockaddrp;
struct sockaddr_in src_addr[MAX_SOCKET_CONNECTION];
pthread_t threads[MAX_SOCKET_CONNECTION], threads_no_alcohol[MAX_SOCKET_CONNECTION], threads_alcohol[MAX_SOCKET_CONNECTION];
int sockfd, conn_fd_list[MAX_SOCKET_CONNECTION]= {0}, car_number;
pthread_mutex_t mutex_server = PTHREAD_MUTEX_INITIALIZER, mutex_str= PTHREAD_MUTEX_INITIALIZER, mutex_file = PTHREAD_MUTEX_INITIALIZER;
ofstream myfile("result.txt");


vector<vector <int> > parking(3 ,vector<int>(8,0));    
vector <int> database[100];
vector <int> result[100];
int numofcar=0;
int fee=0,pos=0,lot=0,grid=0;
int totalfee=0;
char tmp[100];

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

void INT_handler(int signum) {
	myfile.close();
	close(sockfd);
	pthread_exit(NULL);
    exit(0);
}

void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void print(int fd, int curnum){
	cout<<"database:"<<endl;
	for(int i=0;i<numofcar;i++){
		for(int j=0;j<5;j++){
			cout<<database[i][j]<<" ";
		}
		cout<<endl;
	}
}
void show(int fd, int curnum){
	int count1=0,count2=0,count3=0;
	for(int i=0;i<8;i++){
		if(parking[0][i]==0) count1++;
		if(parking[1][i]==0) count2++;
		if(parking[2][i]==0) count3++;
	}
	string s = "";
	char tt[200];
	char c;
	c = count1+'0';
	s = s+"P1: "+c+"\ngrid |";
	for(int i=0;i<8;i++){
		if(parking[0][i]==0){
			c = i+1+'0';
			s = s+c+"|";
		}
	}
	if(count1==0) s = s+"|";
	c = count2+'0';
	s = s+"\nP2: "+c+"\ngrid |";
	for(int i=0;i<8;i++){
		if(parking[1][i]==0){
			c = i+1+'0';
			s = s+c+"|";
		}
	}
	if(count2==0) s = s+"|";
	c = count3+'0';
	s = s+"\nP3: "+c+"\ngrid |";
	for(int i=0;i<8;i++){
		if(parking[2][i]==0){
			c = i+1+'0';
			s = s+c+"|";
		}
	}
	if(count3==0) s = s+"|";
	s = s+"\n"+'\0';
	for(int i=0;i<s.size();i++) tt[i] = s[i];
	write(fd, tt, strlen(tt));
	memset(tt, '\0', 100);
}
bool reserve(int fd, int curnum,int lot,int grid){
	if(parking[lot-1][grid-1]==0){
		parking[lot-1][grid-1] = 1;
		fee +=30;
		database[numofcar].push_back(car_number);
		database[numofcar].push_back(lot-1);
		database[numofcar].push_back(grid-1);
		database[numofcar].push_back(0);
		database[numofcar].push_back(30);
		result[numofcar].push_back(lot);
                result[numofcar].push_back(grid);
                result[numofcar].push_back(car_number);
		numofcar +=1;
		sprintf(tmp, "Reserve successful.");
		write(fd, tmp, strlen(tmp));
		memset(tmp, '\0', 100);
		return true;
	}
	else if(parking[lot-1][grid-1]==1){
		sprintf(tmp, "Error! Please select an ideal grid.");
		write(fd, tmp, strlen(tmp));
		memset(tmp, '\0', 100);
		return false;
	}
	
}

void cancel(int fd, int curnum,int lot,int grid){
	parking[lot-1][grid-1]=0;
	database[pos][0]=0;
	totalfee +=20;
	sprintf(tmp, "Reserve fee: $20.");
	write(fd, tmp, strlen(tmp));
	memset(tmp, '\0', 100);
}

void pick_up(int fd, int curnum,int lot,int grid){
	parking[lot-1][grid-1]=0;
	totalfee +=database[pos][4];
	sprintf(tmp, "Parking fee: $%d.",database[pos][4]);
	write(fd, tmp, strlen(tmp));
	memset(tmp, '\0', 100);
	database[pos][0]=0;
}

bool check_in(int fd, int curnum,int lot,int grid){
	if(parking[lot-1][grid-1]==0){
		parking[lot-1][grid-1] = 1;
		fee +=40;
		database[numofcar].push_back(car_number);
		database[numofcar].push_back(lot-1);
		database[numofcar].push_back(grid-1);
		database[numofcar].push_back(1);
		database[numofcar].push_back(40);
		result[numofcar].push_back(lot);
                result[numofcar].push_back(grid);
                result[numofcar].push_back(car_number);
		numofcar +=1;
		sprintf(tmp, "Check-in successful.");
		write(fd, tmp, strlen(tmp));
		memset(tmp, '\0', 100);
		return true;
	}
	else if(parking[lot-1][grid-1]==1){
		sprintf(tmp, "Error! Please select an ideal grid.");
		write(fd, tmp, strlen(tmp));
		memset(tmp, '\0', 100);
		return false;
	}
}

void *parking_server(void *socket_num){
	char *menu;
	int cur_num = *((int*)(&socket_num));
	int fd = conn_fd_list[cur_num];
	bool entercar = false;
	int parked=0;
	bool isreserve=false,noexit=true;
	char rcv[100];
	char rcv_tmp[100];
	char a[10],b[10];
	fee=0;
	//totalfee=0;
	while(noexit){
		char s[30];
		if(read(fd, rcv, sizeof(rcv)) != -1 && strlen(rcv)>1){
			//cut down command
			//cout<<"rcv: "<<rcv<<endl;
			strcpy(rcv_tmp,rcv);
			menu = strtok(rcv_tmp, " ");
			//cout<<"rcv1: "<<rcv<<endl;
			if(entercar){
				if(parked==0){
					if(isreserve){
						//"show/cancel/check-in/exit";
						if(strncmp(menu,"show",4)==0){
							show(fd, cur_num);
							continue;
						}
						else if(strncmp(menu,"cancel",6)==0) cancel(fd, cur_num,lot,grid);
						else if(strncmp(menu,"check-in",8)==0){
							database[pos][3] = 1;
							sprintf(tmp, "Check-in successful.");
							//sprintf(tmp, "Check-in successful. Your grid is at lot %d grid %d.",lot+1,grid+1);
							write(fd, tmp, strlen(tmp));
							memset(tmp, '\0', 100);
						}
						else if(strncmp(menu,"exit",4)==0){
							sprintf(tmp, "Logout.");
							write(fd, tmp, strlen(tmp));
							memset(tmp, '\0', 100);
							noexit=false;
						}
						else{
							sprintf(tmp, "Invaild command.");
							write(fd, tmp, strlen(tmp));
							memset(tmp, '\0', 100);
							continue;
						}
						//print(fd, cur_num);
					}
					else{
						//"show/reserve/check-in/exit";
						if(strncmp(menu,"show",4)==0){
							show(fd, cur_num);
							continue;
						}
						else if(strncmp(menu,"reserve",7)==0){
							sscanf(rcv, "reserve %s %s", a,b);
							int aa = atoi(a);
							int bb = atoi(b);
							bool re;
							if(0<aa && aa<4 && 0<bb && bb<9){
								lot = aa+1;
								grid = bb+1;
								re = reserve(fd, cur_num,aa,bb);
								if(!re){
									memset(rcv, '\0', 100);
                                                                        memset(rcv_tmp, '\0', 100);
									memset(a, '\0', 10);
                                                                        memset(b, '\0', 10);
									continue;
								}
							}
							else{
								sprintf(tmp, "Invaild command.");
								write(fd, tmp, strlen(tmp));
								memset(tmp, '\0', 100);
								memset(rcv, '\0', 100);
                                                                memset(rcv_tmp, '\0', 100);
                                                                memset(a, '\0', 10);
								memset(b, '\0', 10);
								continue;
							}
						}
						else if(strncmp(menu,"check-in",8)==0){
							sscanf(rcv, "check-in %s %s", a,b);
							int aa = atoi(a);
							int bb = atoi(b);
							if(0<aa && aa<4 && 0<bb && bb<9){
                                                        	bool ck;
								ck = check_in(fd, cur_num,aa,bb);
								if(!ck){
									memset(rcv, '\0', 100);
									memset(rcv_tmp, '\0', 100);
									memset(a, '\0', 10);
                                                                        memset(b, '\0', 10);
									continue;
								}
							}
							else{
								sprintf(tmp, "Invaild command.");
								write(fd, tmp, strlen(tmp));
								memset(tmp, '\0', 100);
								memset(rcv, '\0', 100);
                                                                memset(rcv_tmp, '\0', 100);
                                                                memset(a, '\0', 10);                                                                        
								memset(b, '\0', 10);
								continue;
							}
						}
						else if(strncmp(menu,"exit",4)==0){
							sprintf(tmp, "Logout.");
							write(fd, tmp, strlen(tmp));
							memset(tmp, '\0', 100);
							noexit=false;
						}
						else{
							sprintf(tmp, "Invaild command.");
							write(fd, tmp, strlen(tmp));
							memset(tmp, '\0', 100);
							memset(rcv, '\0', 100);
                                                        memset(rcv_tmp, '\0', 100);
                                       	                memset(a, '\0', 10);
                                                        memset(b, '\0', 10);
							continue;
						}
						//print(fd, cur_num);
					}
					entercar = false;
				}
				else if(parked==1){
					//"show/pick-up";
					if(strncmp(menu,"show",4)==0){
						show(fd, cur_num);
						continue;
					}
					else if(strncmp(menu,"pick-up",7)==0){
						pick_up(fd, cur_num,lot,grid);
					}
					else{
						sprintf(tmp, "Invaild command.");
						write(fd, tmp, strlen(tmp));
						memset(tmp, '\0', 100);
						memset(rcv, '\0', 100);
                                                memset(rcv_tmp, '\0', 100);
						continue;
					}
					//print(fd, cur_num);
					entercar = false;
				}
			}
			else{
				if(sprintf(s, "%d", atoi(menu))!=4){
					sprintf(tmp, "Invaild command.");
					write(fd, tmp, strlen(tmp));
					memset(tmp, '\0', 100);
					continue;
				}
				car_number = atoi(menu);
				//cout<<"car_number: "<<car_number<<endl;
				entercar = true;
				isreserve = false;
				for(int i=0;i<numofcar;i++){
					if(car_number == database[i][0]){
						pos = i;
						isreserve=true;
						lot = database[i][1]+1;
						grid = database[i][2]+1;
						parked = database[i][3];
						fee = database[i][4];
						break;
					}
				}
				if(parked==1){
					sprintf(tmp, "Your grid is at lot P%d grid %d.",lot,grid);
					write(fd, tmp, strlen(tmp));
					memset(tmp, '\0', 100);
				}
				else{
					if(isreserve){
						sprintf(tmp, "You have reserved grid.");
						write(fd, tmp, strlen(tmp));
						memset(tmp, '\0', 100);
	
					}
					else{
						sprintf(tmp, "You haven't reserved grid.");
						write(fd, tmp, strlen(tmp));
						memset(tmp, '\0', 100);
					}
				}
			}

		}
		else{
			scanf("%s",&a);
			if(strncmp(a,"end",3)==0){
				for(int i=0;i<numofcar;i++){
					myfile<<result[i][0]<<","<<result[i][1]<<","<<result[i][2]<<endl;
                                	cout<<result[i][0]<<","<<result[i][1]<<","<<result[i][2]<<endl;
				}
				sprintf(tmp,"Total income: %d\n",totalfee);
        			myfile << tmp;
        			cout << tmp;
				myfile.close();
			}
		}
		memset(rcv, '\0', 100);
		memset(rcv_tmp, '\0', 100);
		memset(tmp, '\0', 100);
		memset(a, '\0', 10);
		memset(b, '\0', 10);
	}
	return 0;
}

int main(int argc,char *argv[]){

	if(argc<2){
        cout<<"please enter correct input <port>\n";
        exit(1);
    }

    //initial socket
	socklen_t src_len = sizeof(src_addr[0]);
	int num_conn = 0;
	int connfd;
    sockfd = server_init(argv[1], "127.0.0.1");

    //signal(SIGCHLD, zombie_handler);
    signal(SIGINT, INT_handler);

    while(1){
		connfd = accept(sockfd, (sockaddrp)&src_addr[num_conn], &src_len);
		if(connfd != -1) {
			conn_fd_list[num_conn] = connfd;
			//cout << "new client"<< endl;
			if(pthread_create(&threads[num_conn], NULL, parking_server, (void *)num_conn)){
				errexit("thread create failed: %s\n", strerror(errno));
			}
			num_conn++;

		}
		//usleep(1);

	}
	perror("Too many connections!");
    close(sockfd);
    return 0;
}
// Vector
// car_num lot grid park fee
/**

You haven't / have reserved grid.
1. show
2. reserve / cancel
3. check-in
4. exit

Your grid is at lot p1 grid 3.
1. show
2. pick-up
**/
