APP = account_service
CC = gcc
AR = ar

#寻找所有的子目录，并跳过debug obj  bin include目录
SUB_DIR = $(shell ls -l | grep ^d | awk '{if(\
                $$9 != "debug" && \
                $$9 != "obj" && \
                $$9 != "bin" && \
                $$9 != "include" &&\
                $$9 != "data"  \
                ) print $$9}')

TOP_DIR = $(PWD)

INC_DIR = $(TOP_DIR)/include

TAG = -lpthread -Wall -lsqlite3

OBJS = $(TOP_DIR)/obj

BIN = $(TOP_DIR)/bin

#寻找当前目录下的所有.c文件
CUR_SRC = ${wildcard *.c}

#把所有的.c文件转换成.o文件
CUR_OBJ = ${patsubst %.c,%.o,$(CUR_SRC)}

#申明环境变量，方便子层Makefile使用
export CC BIN AR TOP_DIR OBJS BIN INC_DIR TAG

all : $(SUB_DIR) $(APP)

#根据SUB_DIR一个个调用子目录的Makfile（make -C） 不知道为啥要加一个echo命令，可能需要对应的tag 
$(SUB_DIR):ECHO
	make -C $@

#将所有的.o编译成APP
$(APP) :
	$(CC) $(OBJS)/*.o -o $@ -I $(INC_DIR) $(TAG)

ECHO:
	echo $(SUB_DIR)    

clean:
	@rm $(OBJS)/* $(APP) -rf
