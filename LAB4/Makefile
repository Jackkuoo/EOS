CC = arm-linux-gnueabihf-gcc
CPLUS = arm-linux-gnueabihf-g++
FLAGS = -g -Wall --static

XLNX_DIR = /home/jackkuo/zc702/linux-xlnx
obj-m = mydev.o

all: mydev writer reader

mydev:
	make -C $(XLNX_DIR) M=$(PWD) modules

%.o: %.cpp
	$(CPLUS) -c $< $(FLAGS)

writer: writer.o
	$(CPLUS) -o $@ $^ $(FLAGS)

reader: reader.o
	$(CPLUS) -o $@ $^ $(FLAGS)

clean:
	make -C $(XLNX_DIR) M=$(PWD) clean
	rm -rf writer writer.o
	rm -rf reader reader.o

.PYONY: all mydev clean
