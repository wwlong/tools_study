# TOP_PATH = ${shell pwd}
TOP_PATH=.
RK3288 = "RK3288"
RK3328 = "RK3328"
HI3519 = "HI3519"
PC = "PC"

# ARCH = "RK3328"
ARCH = "RK3288"
# ARCH = "PC"
#CROSS = arm-hisiv600-linux-gnueabi-
CROSS = 


ifeq (${ARCH}, ${RK3288}) 
	CROSS := arm-linux-gnueabihf-
else ifeq (${ARCH}, ${HI3519}) 
	CROSS := arm-hisiv600-linux-gnueabi-
else ifeq (${ARCH}, ${RK3328}) 
	# CROSS := aarch64-linux-gnu-
else ifeq (${ARCH}, ${PC}) 
	CROSS := 
endif 

# RK3288
AR                    := ${CROSS}ar
C_COMPILER            := ${CROSS}gcc
CPLUSPLUS_COMPILER    := ${CROSS}g++


PROJECT_DIR 		:= ${TOP_PATH}
PROJECT_BUILD_DIR   := `pwd`
SNFACE_SDK_DIR		:= .
TARGET      		:= ./build/globdemo
TARGETPATH  		:= .

#源码目录
SOURCEDIRLIST	+=	${TOP_PATH}/demo
# 头文件
INCLUDEDIRS		+=	-I${TOP_PATH}/include

ifeq (${ARCH}, ${RK3288}) 
	COMPILE_FLAGS += -mfloat-abi=hard -mfpu=neon-vfpv4 -mcpu=cortex-a15 -ffast-math -ftree-vectorize
else ifeq (${ARCH}, ${RK3328}) 

endif

COMPILE_FLAGS += -std=c++11 -O3 -D _GNU_SOURCE

CPLUSPLUS_SOURCES 		:= $(foreach SUBDIR,$(SOURCEDIRLIST),$(wildcard $(SUBDIR)/*.cpp))
C_SOURCES			    := $(foreach SUBDIR,$(SOURCEDIRLIST),$(wildcard $(SUBDIR)/*.c))

#忽略文件
_SOURCES        := 

OBJDIR          := objs
OBJDIRLIST 	    := $(foreach SUBDIR,$(SOURCEDIRLIST),$(OBJDIR)/$(subst $(PROJECT_DIR)/,,$(SUBDIR)))

CPLUSPLUS_SOURCES     	:= $(filter-out $(_SOURCES),$(CPLUSPLUS_SOURCES))
C_SOURCES     			:= $(filter-out $(_SOURCES),$(C_SOURCES))

CPLUSPLUS_OBJS        	:= $(patsubst %.cpp,$(OBJDIR)/%.o,$(subst $(PROJECT_DIR)/,,$(CPLUSPLUS_SOURCES)))
C_OBJS        			:= $(patsubst %.c,$(OBJDIR)/%.o,$(subst $(PROJECT_DIR)/,,$(C_SOURCES)))

CPLUSPLUS_DEPENDENCE        := $(CPLUSPLUS_OBJS:.o=.cpp.dep)
C_DEPENDENCE                := $(C_OBJS:.o=.c.dep)

#COMPILE_FLAGS   = -Og -g -Wall -ffunction-sections -fdata-sections -pipe $(DEFINES) -fPIC
LINK_FLAGS      = -Og -g -Wl,--gc-sections $(LIBS) -pthread -lm -ldl -lrt -L./lib -lglog
COMPILE_FLAGS   += -Og  -Wall $(DEFINES)
LINK_FLAGS += $(LIBS)

all: $(TARGET)

$(TARGET): $(CPLUSPLUS_OBJS) $(C_OBJS)
	$(CPLUSPLUS_COMPILER) $^ $(LINK_FLAGS) -o $@
#	cp $(TARGET) $(TARGETPATH)

$(CPLUSPLUS_OBJS): $(OBJDIR)/%.o: $(PROJECT_DIR)/%.cpp
	$(CPLUSPLUS_COMPILER) $(COMPILE_FLAGS) $(INCLUDEDIRS) -std=c++11 -o $@ -c $<

$(C_OBJS): $(OBJDIR)/%.o: $(PROJECT_DIR)/%.c
	$(C_COMPILER) $(COMPILE_FLAGS) $(INCLUDEDIRS) -o $@ -c $<

clean:
	$(RM) $(CPLUSPLUS_OBJS) $(C_OBJS) $(TARGET) $(CPLUSPLUS_DEPENDENCE) $(C_DEPENDENCE)
	${RM} $(PROJECT_DIR)/build/objs -rf
#依赖
sinclude $(CPLUSPLUS_DEPENDENCE)
sinclude $(C_DEPENDENCE)

$(CPLUSPLUS_DEPENDENCE): $(OBJDIR)/%.cpp.dep: $(PROJECT_DIR)/%.cpp
	mkdir -p $(OBJDIRLIST)
	$(CPLUSPLUS_COMPILER) -MM -std=c++11 $(COMPILE_FLAGS) $(INCLUDEDIRS) $< > $@.tmp
	sed 's,\([a-zA-Z0-9_]*\)\.o[ :]*,$(OBJDIR)/$*\.o $@ : ,g' < $@.tmp > $@
	$(RM) $@.tmp

$(C_DEPENDENCE): $(OBJDIR)/%.c.dep: $(PROJECT_DIR)/%.c
	mkdir -p $(OBJDIRLIST)
	$(C_COMPILER) -MM $(COMPILE_FLAGS) $(INCLUDEDIRS) $< > $@.tmp
	sed 's,\([a-zA-Z0-9_]*\)\.o[ :]*,$(OBJDIR)/$*\.o $@ : ,g' < $@.tmp > $@
#	sed 's,\($*\)\.o[ :]*,\1.o $@ ,g' < $@.tmp > $@
	$(RM) $@.tmp
