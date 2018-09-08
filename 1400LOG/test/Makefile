# 依赖文件
# DG_ESE_DIR = /workspace/documents/deepglint/AIBox/sdk/ese/dg_ese_sdk
# CROSS = arm-linux-gnueabihf-
CC = ${CROSS}gcc
CXX = ${CROSS}g++
RM = rm
# SRC_CPP += ./tracker/lh_tracking.cpp 
# SRC_CPP += ./tracker/Tracker.cpp
# log
SRC_CPP += ../1400log.cpp 
SRC_CPP += ./main.cpp
INC += -I../
# INC += -I./tracker
CPPFLAGS += -g
CPPFLAGS += ${INC} 
TARGET = DG_LOG_TEST
ALL:${TARGET}
${TARGET}:${SRC_CPP}
	${CXX} ${CPPFLAGS} $^ -o $@ ${LIBS}

.PHONY:clean
clean:
	${RM} ${TARGET} *~ ./-O3 -rf
