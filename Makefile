APP = account_service
CC = gcc
AR = ar

#Ѱ�����е���Ŀ¼��������debug obj  bin includeĿ¼
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

#Ѱ�ҵ�ǰĿ¼�µ�����.c�ļ�
CUR_SRC = ${wildcard *.c}

#�����е�.c�ļ�ת����.o�ļ�
CUR_OBJ = ${patsubst %.c,%.o,$(CUR_SRC)}

#�������������������Ӳ�Makefileʹ��
export CC BIN AR TOP_DIR OBJS BIN INC_DIR TAG

all : $(SUB_DIR) $(APP)

#����SUB_DIRһ����������Ŀ¼��Makfile��make -C�� ��֪��ΪɶҪ��һ��echo���������Ҫ��Ӧ��tag 
$(SUB_DIR):ECHO
	make -C $@

#�����е�.o�����APP
$(APP) :
	$(CC) $(OBJS)/*.o -o $@ -I $(INC_DIR) $(TAG)

ECHO:
	echo $(SUB_DIR)    

clean:
	@rm $(OBJS)/* $(APP) -rf
