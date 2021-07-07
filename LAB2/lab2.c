//-------------------------------------------------------------------//
// Keyboard input LCD sync
// 7-Segment output
// A + B - C * D / * Clear # =
// arm-unknown-linux-gnu-gcc -o lab2 lab2.c \
//	-L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ \
//	-I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ \
//	-I /home/jackkuo/microtime/linux/include/
//
// (C) Copyright ChunTing Kuo 2020/10/29
//-------------------------------------------------------------------//

#include <stdio.h> // perror()
#include <stdlib.h> // exit()
#include <sys/fcntl.h> // open()
#include <sys/ioctl.h>
#include <unistd.h> // close()
#include <string.h>
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"


int stack[1000];
int top=-1;
int isEmpty();
void push(int); 
int pop();

int isEmpty(){
	if(top==-1){
		return 1; 
	}else{
		return 0;
	}
}

void push(int data){
	if(top>=MAXSTACK){
		printf("FULL\n");	
	}else{
		top++;
		stack[top]=data;
	}
 
} 

int pop(){
	int data;
	if(isEmpty()){
		printf("EMPTY\n");
	}else{
		data=stack[top];
		top--;
		return data; 
	}
}

int main(){
	
	int fd;
	lcd_write_info_t display; //struct for saving data

	if((fd = open("/dev/lcd", O_RDWR)) < 0 ) {
		printf("open_/dev/lcd_error\n");
		return (-1);
	}
	
	// Keyboard //-------------------
	unsigned short key;
	int fd_key,ret_key;
	ioctl(fd_key , KEY_IOCTL_CLEAR, key);
	//---------------------------------
	// 7-Segment //-------------------
	_7seg_info_t data;
	int fd_7seg,ret_7seg,i;
	ioctl(fd_7seg , _7SEG_IOCTL_ON, NULL);
	//---------------------------------
	// LCD //-------------------
	ioctl(fd , LCD_IOCTL_CLEAR, NULL);
	//---------------------------------
	while (1) {
		ret_key = ioctl(fd_key , KEY_IOCTL_CHECK_EMTPY, &key ) // Keyboard
		if(ret_key < 0){
 			sleep (1);
 			continue;
		}
		string c;
		char op;
 		ret_key = ioctl(fd_key , KEY_IOCTL_GET_CHAR, &key) // Keyboard
 		if((key & 0xff) == '#'){
 			int calculate;
 			while(! isEmpty()){
 				calculate += pop();
 			}
 			display.Count = sprintf((char∗)display.Msg,calculate);
			ioctl(fd , LCD_IOCTL_WRITE, &display);
 		}
 		else if((key & 0xff) == '*') break;
 		else if((key & 0xff) == 'A'){
 			op = "+";
 			display.Count = sprintf((char∗)display.Msg,"+\n");
			ioctl(fd , LCD_IOCTL_WRITE, &display);
 		}
		else if((key & 0xff) == 'B'){
			op = "-";
			display.Count = sprintf((char∗)display.Msg,"-\n");
			ioctl(fd , LCD_IOCTL_WRITE, &display);
		}
		else if((key & 0xff) == 'C'){
			op = "*";
			display.Count = sprintf((char∗)display.Msg,"*\n");
			ioctl(fd , LCD_IOCTL_WRITE, &display);
		}
		else if((key & 0xff) == 'D'){
			op = "/";
			display.Count = sprintf((char∗)display.Msg,"/\n");
			ioctl(fd , LCD_IOCTL_WRITE, &display);
		}
		else{
			char num = (key & 0xff);
			int number = num - '0';
			string displayled = "0x";
			if(c.size() < 4){
				int zero = 4 - c.size();
				for(int i=0;i<zero;i++) displayled += "0"
			}
			for(int i=0;i<c.size();i++){
				displayled += c[i];
			}
			displayled += num;
			c += num;
			data.Mode = _7SEG_MODE_HEX_VALUE;
			data.Which = _7SEG_ALL;
			data.Mode = displayled;
			ioctl(fd_7seg, _7SEG_IOCTL_SET, &data);
			sleep(3);

			if (op == "+") push(number);
			else if (op == "-") push(number * (-1));
			else if (op == "*") push(pop() * number)
			else if (op == "/") push(pop() / number)
		}
	}


	close(fd_7seg)
	close(fd_key);
	close(fd);
	return 0;
}


