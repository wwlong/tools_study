export PROJECT_DIR := $(shell pwd)

#CROSS_COMPILE = arm-hisiv400-linux-
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
CXX             = $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
RANLIB		= $(CROSS_COMPILE)ranlib
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM RANLIB
export STRIP OBJCOPY OBJDUMP

#CFLAGS += -Wall -O3 -g -std=c++11 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mcpu=cortex-a7
CFLAGS += -Wall -O3 -g -std=c++11
CFLAGS += -I $(PROJECT_DIR)

export CFLAGS

LIBFTP=libftp.a
OBJS = FtpClientLib.o

ALL : libftp.a ftpTest 
%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

libftp.a : $(OBJS)
	$(AR) rc $(LIBFTP) $(OBJS) 
ftpTest :
	${CXX} ./test.c  -I./ ${CFLAGS} -o $@ ${LIBFTP}  
clean:
	rm -f $(LIBFTP)
	rm -f $(shell find -name "*.o")
