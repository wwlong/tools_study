SRC += ./main.c 
SRC += ../msgq.c
SRC += ../RingBuf.cpp 
INC += -I../ 
CFLAGS +=  -g -W -Wall -I../..  -I./ -Wno-unused-function -Wunused-parameter
CFLAGS += ${INC} 
LIBS += -lpthread  -lSDL2 -lSDL2_image 
CC = gcc
	
msgq_demo:${SRC}
	${CC}  ${CFLAGS} $^ -o $@ ${LIBS}
.PHONY:clean
clean:
	rm msgq_demo -rf

