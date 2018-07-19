#CROSS = arm-linux-gnueabihf-
CC = ${CROSS}gcc
CPPFLAGS += -g
LIBS += -lrt 

SRC += ./shm_read.c 

shm_read:${SRC}
	${CC} ${CPPFLAGS} $^ -o $@ ${LIBS} 

.PHONY:clean
clean:
	@${RM} shm_read *.~ -rf
