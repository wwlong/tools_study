CC = gcc
CPPFLAGS += -g
LIBS += -lrt 

SRC += ./shm_write.c  

shm_write:${SRC}
	${CC} ${CPPFLAGS} $^ -o $@ ${LIBS} 

.PHONY:clean
clean:
	@${RM} shm_write *.~ -rf
