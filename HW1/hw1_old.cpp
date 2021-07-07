#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

using namespace std;

#define _7SEG_MODE_PATTERN 0
#define _7SEG_MODE_HEX_VALUE 1 

#define _7SEG_D5_INDEX 8 // Segment D5 (1)
#define _7SEG_D6_INDEX 4 // Segment D6 (2)
#define _7SEG_D7_INDEX 2 // Segment D7 (3)
#define _7SEG_D8_INDEX 1 // Segment D8 (4)
#define _7SEG_ALL (_7SEG_D5_INDEX|_7SEG_D6_INDEX|_7SEG_D7_INDEX|_7SEG_D8_INDEX)

vector<vector <int> > parking(3 ,vector<int>(8,0));    
vector <int> database[100];
int numofcar=0;
int fee=0,car_number=0,pos=0,lot=0,grid=0,fd,ret;
unsigned short key, segnum, leddata;
lcd_write_info_t display;
_7seg_info_t data;

void LED(int);
unsigned long segm(int);
void show();
void reserve();
void cancel(int lot,int grid);
void pick_up(int lot,int grid);
void check_in();
void LED(int parknum){
	for(int k=0;k<8;k++){
		ioctl(fd, LED_IOCTL_BIT_SET, &parking[parknum][k]);
	}
}

unsigned long segm(int ans){
	char buffer[10];
	int a = ans;
	int b = a%10;
 	int c = (a%100-b)/10;
	int d = (a%1000-10*c-b)/100;
	int e = (a-100*d-10*c-b)/1000;
	int h = b+c*16+d*(16*16)+e*(16*16*16);
	sprintf(buffer,"%x",h);
	unsigned int car_num = strtoul(buffer,NULL,16);
	printf("%u\n",car_num);
	return car_num;
}

void show(){
	int count1=0,count2=0,count3=0;
	for(int i=0;i<8;i++){
		if(parking[0][i]==0) count1++;
		if(parking[1][i]==0) count2++;
		if(parking[2][i]==0) count3++;
	}
	display.Count = sprintf((char*)display.Msg," p1 %d\n p2 %d\n p3 %d\n",count1,count2,count3);
	ioctl(fd,LCD_IOCTL_WRITE,&display);
}
void reserve(){
	//cout<<"Select parking lot: ";
	//cin>>lot;
	//cout<<"Select parking grid: ";
	//cin>>grid;
	display.Count = sprintf((char*)display.Msg,"Select parking lot: ");
	ioctl(fd,LCD_IOCTL_WRITE,&display);
	ret = ioctl(fd, KEY_IOCTL_WAIT_CHAR, &key);
	lot = (char)key-'0';
	display.Count = sprintf((char*)display.Msg,"\nSelect parking grid: ");
	ioctl(fd,LCD_IOCTL_WRITE,&display);
	ret = ioctl(fd, KEY_IOCTL_WAIT_CHAR, &key);
	grid = (char)key-'0';
	if(parking[lot-1][grid-1]==0){
		parking[lot-1][grid-1] = 1;
		fee +=30;
		database[numofcar].push_back(car_number);
		database[numofcar].push_back(lot-1);
		database[numofcar].push_back(grid-1);
		database[numofcar].push_back(0);
		database[numofcar].push_back(30);
		numofcar +=1;
		cout<<"Have a nice day!"<<endl;
		display.Count = sprintf((char*)display.Msg,"Have a nice day!\n");
		ioctl(fd,LCD_IOCTL_WRITE,&display);
	}
	else if(parking[lot-1][grid-1]==1){
		cout<<"Error! Please select an ideal grid. Return menu."<<endl;
		display.Count = sprintf((char*)display.Msg,"Error! Please select an ideal grid. Return menu.\n");
		ioctl(fd,LCD_IOCTL_WRITE,&display);
	}
	
}
void cancel(int lot,int grid){
	parking[lot-1][grid-1]=0;
	database[pos][0]=0;
	cout<<"lot: "<<lot<<"grid: "<<grid<<endl;
	cout<<"Parking fee: $20"<<endl;
	display.Count = sprintf((char*)display.Msg,"Parking fee: $20\n");
	ioctl(fd,LCD_IOCTL_WRITE,&display);
}
void pick_up(int lot,int grid){
	parking[lot-1][grid-1]=0;
	cout<<"Parking fee: $"<<database[pos][4]<<endl;
	display.Count = sprintf((char*)display.Msg,"Parking fee: $%d\n",database[pos][4]);
	ioctl(fd,LCD_IOCTL_WRITE,&display);
	database[pos][0]=0;
}
void check_in(int lot,int grid){
	if(parking[lot-1][grid-1]==0){
		parking[lot-1][grid-1]=1;
		fee+=40;
		database[numofcar].push_back(car_number);
                database[numofcar].push_back(lot-1);
                database[numofcar].push_back(grid-1);
                database[numofcar].push_back(1);
                database[numofcar].push_back(40);
		display.Count = sprintf((char*)display.Msg,"Check-in Successful");
		ioctl(fd,LCD_IOCTL_WRITE,&display);
	}
	numofcar+=1;
	else if(parking[lot-1][grid-1]==1){
		display.Count = sprintf((char*)display.Msg,"Error! Please select an ideal grid.");
		ioctl(fd,LCD_IOCTL_WRITE,&display);
	}
}
int main(){
	int menu,parked=0,cal;
	bool isreserve=false,noexit=true;

	/* Open device /dev/lcd */
 	if((fd = open("/dev/lcd",O_RDWR)) < 0)
  	{  
    	printf("open /dev/lcd failed\n");
    	return(-1);
  	}
	ioctl(fd , _7SEG_IOCTL_ON, NULL);
	ioctl(fd, KEY_IOCTL_CLEAR, key);
	ioctl(fd ,LCD_IOCTL_CLEAR, NULL);
	leddata = LED_ALL_OFF;
	ioctl(fd, LED_IOCTL_SET, &leddata);
	while(noexit){
		ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY,&key);
		if(ret < 0){
			sleep (1);
			continue;
		}   
		ret = ioctl(fd, KEY_IOCTL_WAIT_CHAR, &key);
		//cin>>car_number;
		int lock=1;
		char s[30];
		cout<<"input plate num"<<endl;
		while(lock){
			display.Count = sprintf((char*)display.Msg,"Input plate num:\n");
			ioctl(fd,LCD_IOCTL_WRITE,&display);
			car_number=0;
			for(int i=0;i<50;i++){
				ret = ioctl(fd, KEY_IOCTL_WAIT_CHAR, &key);
				cal = (char)key;
				printf("cal=%c\n",cal);
				display.Count = sprintf((char*)display.Msg,"%c",cal);
				ioctl(fd,LCD_IOCTL_WRITE,&display);
				if(cal == '#'){
					car_number = car_number/10;
					break;
				}
				else if(cal < '10' && cal >= '0'){
					int a = (cal-'0');
					car_number += a;
					cout<<"a= "<<a<<endl;
					cout<<"carnumber= "<<car_number<<endl;
					car_number *= 10;
				}
				else{
					display.Count = sprintf((char*)display.Msg,"Input Error\n");
                                	ioctl(fd,LCD_IOCTL_WRITE,&display);
				}
			}
			if(sprintf(s, "%d", car_number)!=4){
				display.Count = sprintf((char*)display.Msg,"Input Error\n");
				ioctl(fd,LCD_IOCTL_WRITE,&display);
				lock =1;
				continue;
			}
			else if(sprintf(s, "%d", car_number)==4){
				lock =0;
			}
		}
		isreserve=false;
		fee=0;
		//7seg
		segnum = segm(car_number);
		data.Mode = _7SEG_MODE_HEX_VALUE;
		data.Which = _7SEG_ALL;
		data.Value = segnum;
		ioctl(fd, _7SEG_IOCTL_SET, &data) ;

		for(int i=0;i<numofcar;i++){
			if(car_number == database[i][0]){
				pos = i;
				isreserve=true;
				lot = database[i][1];
				grid = database[i][2];
				parked = database[i][3];
				fee = database[i][4];
				break;
			}
		}
		if(parked==0){
			if(isreserve){
				cout<<"444444\n"<<endl;
				display.Count = sprintf((char*)display.Msg,"\nYou have reserved grid.\n 1. show\n 2. cancel\n 3. check-in\n 4. exit\n");
				ioctl(fd,LCD_IOCTL_WRITE,&display);
				ret = ioctl(fd, KEY_IOCTL_WAIT_CHAR, &key);
				menu = (char)key-'0';
				//cin>>menu;
				LED(lot);
				if(menu==1) show();
				else if(menu==2) cancel(lot,grid);
				else if(menu==3){
					database[pos][3] = 1;
					//cout<<"Your grid is at lot p"<<lot+1<<" grid "<<grid+1<<endl;
					display.Count = sprintf((char*)display.Msg,"\nYour grid is at lot p%d grid %d\n 1. show\n 2. pick-up\n",lot+1,grid+1);
					ioctl(fd,LCD_IOCTL_WRITE,&display);
				}
				else if(menu==4) noexit=false;
				else{
					//cout<<"Input Error"<<endl;
					display.Count = sprintf((char*)display.Msg,"Input Error");
					ioctl(fd,LCD_IOCTL_WRITE,&display);
        			continue;
				}
			}
			else{
				cout<<"555555\n"<<endl;
				display.Count = sprintf((char*)display.Msg,"\nYou haven't reserved grid.\n 1. show\n 2. reserve\n 3. check-in\n 4. exit\n");
				ioctl(fd,LCD_IOCTL_WRITE,&display);
				ret = ioctl(fd, KEY_IOCTL_WAIT_CHAR, &key);
				menu = (char)key-'0';
				//cin>>menu;
				if(menu==1) show();
				else if(menu==2) reserve();
				else if(menu==3) check_in();
				else if(menu==4) noexit=false;
				else{
					display.Count = sprintf((char*)display.Msg,"Input Error");
					ioctl(fd,LCD_IOCTL_WRITE,&display);
        			continue;
				}
			}
		}
		else if(parked==1){
			cout<<"666666\n"<<endl;
			display.Count = sprintf((char*)display.Msg,"\nYour grid is at lot p%d grid %d\n 1. show\n 2. pick-up\n",lot+1,grid+1);
			ioctl(fd,LCD_IOCTL_WRITE,&display);
			LED(lot);
			//cin>>menu;
			menu = (char)key-'0';
			if(menu==1) show();
			else if(menu==2) pick_up(lot,grid);
			else{
				display.Count = sprintf((char*)display.Msg,"Input Error\n");
				ioctl(fd,LCD_IOCTL_WRITE,&display);
        		continue;
			}
		}
	}
	close(fd);
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
