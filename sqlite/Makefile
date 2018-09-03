SUB_DIR = $(shell ls -l | grep ^d | awk '{print $$9}')
CUR_SRC = ${wildcard *.c}
CUR_OBJS = ${patsubst %.c,%.o,$(CUR_SRC)}

all:$(SUB_DIR) $(CUR_OBJS)

$(SUB_DIR):ECHO
	make -C $@
ECHO:
	echo $(SUB_DIR)

$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(OBJS)/$@ -I $(INC_DIR)/
