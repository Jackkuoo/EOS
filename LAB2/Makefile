CC := arm-unknown-linux-gnu-gcc
exe := lab2
obj := lab2.c
mkfile_path := $(shell pwd)

all:$(obj)
	$(CC) -o $(exe) $(obj) -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I $(mkfile_path)/microtime/linux/include/

clean:
	rm -rf lab2
