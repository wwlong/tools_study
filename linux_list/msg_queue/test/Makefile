#CROSS = arm-linux-gnueabihf-
CXX = ${CROSS}g++
RM = rm
STRIP = ${CROSS}strip

SRTIPFLAGS = --strip-unneeded
AR 	= ${CROSS}ar
ARFLAGS = rc

SRCCPP += ../msgq.c  

TESTSRC += ./main.c 
#TESTSRC += ${SRCC}
#TESTSRC += ${SRCCPP}

INC += -I../  
#CPPFLAGS +=  -mfloat-abi=hard -mfpu=neon-vfpv4 -march=cortex-a15 -ffast-math -ftree-vectorize  -std=c++11 -fmessage-length=0 -MMD -MP -MF  -O3 -DNDEBUG   -fopenmp -rdynamic
CPPFLAGS +=  -g -W -Wall -Wno-unused-function -Wunused-parameter -fpermissive
CPPFLAGS += ${INC} 
LIBS += -lpthread

TEMP = ${SRCCPP}

OBJ_SRCCPP = $(TEMP:.cpp=.o)

OBJ_SRCCPP_U = $(TEMP:.c=.u)

%.o:%.c
	${CXX} ${CPPFLAGS} -c $< -o $@

OBJ_SRC += ${OBJ_SRCCPP}
ALL:msgq_demo 

#dgmsgq.a:${OBJ_SRC}
#	$(AR) $(ARFLAGS) $@ $^
#	#@${RM} -rf ${OBJ_SRC} 
#	$(STRIP) ${SRTIPFLAGS} $@
#
#LIBS += ./dgmsgq.a
TESTSRC += ${SRCCPP} 
msgq_demo:${TESTSRC}
	${CXX}  ${CPPFLAGS} $^ -o $@ ${LIBS}

.PHONY:clean
clean:
	@${RM} msgq_demo *~ dgmsgq.a -rf

