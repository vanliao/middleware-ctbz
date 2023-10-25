CC := g++
CFLAGS := -std=c++11 -O2 -g -I log/ -I msg/ -I network/ -I service/ -I db/ -I dev/ -I main/ -I public/ -I thirdparty/xml/ -I thirdparty/inifile/ -I thirdparty/rapidjson/  -I thirdparty/fcgi -I thirdparty/sqlite3 -I thirdparty

ifeq ($(platform), arm)
CC := aarch64-linux-gnu-g++
CFLAGS := -std=c++11 -DARM -O2 -g -Wno-psabi -I log/ -I msg/ -I network/ -I service/ -I db/ -I dev/ -I main/ -I public/ -I thirdparty/xml/ -I thirdparty/inifile/ -I thirdparty/rapidjson/ -I thirdparty/fcgi -I thirdparty/sqlite3 -I thirdparty
endif

# define source path
OUTPUT_DIR    := ./output
EXECUTABLE    := ./middleware

OUTPUT_OBJ_DIR := ${OUTPUT_DIR}/obj


SOURCE := $(wildcard ./main/*.cpp) \
		$(wildcard ./dev/*.cpp) \
		$(wildcard ./thirdparty/inifile/*.cpp) \
		$(wildcard ./log/*.c) \
		$(wildcard ./log/*.cpp) \
		$(wildcard ./msg/*.cpp) \
		$(wildcard ./network/*.cpp) \
		$(wildcard ./service/*.cpp) \
		$(wildcard ./thirdparty/xml/*.cpp) \
		$(wildcard ./public/*.cpp) \
		$(wildcard ./db/*.cpp)

DYNAMIC_LIBS = -L ./lib/arm -L ./lib/ubuntu -lpthread -lfcgi -lsqlite3 -lssl -lcrypto -ldl

C_OBJS := $(filter %.c, $(SOURCE))
CPP_OBJS := $(filter %.cpp, $(SOURCE))
SOURCE_OBJS  := $(patsubst %.cpp,${OUTPUT_OBJ_DIR}/%.o,$(notdir $(CPP_OBJS))) 
SOURCE_OBJS  += $(patsubst %.c,${OUTPUT_OBJ_DIR}/%.o,$(notdir $(C_OBJS))) 
OBJS :=  ${SOURCE_OBJS} 

HD_VERSION=$(shell echo "\#define HARDWARE_VERSION \"v1.0\"">main/version.h)
OS_VERSION=$(shell echo "\#define OS_VERSION \"MIPS\"">>main/version.h)
CURR_SOFT_VERSION=$(shell echo "\#define CURRENT_SOFT_VERSION \"v1.0\"">>main/version.h)
ORIGINAL_SOFT_VERSION=$(shell echo "\#define ORIGINAL_SOFT_VERSION \"v0.0\"">>main/version.h)
SOFT_UPDATE_TIME = $(shell date +%s)
SOFT_UPDATE=$(shell echo "\#define SOFT_UPDATE_TIME ${SOFT_UPDATE_TIME}">>main/version.h)
VERSION_NUM=$(shell echo "\#define VERSION_NUM ${SOFT_UPDATE_TIME} / 3600">>main/version.h)
CHMOD_VERSION=$(shell chmod 755 main/version.h)

.PHONY : all clean outPrint 

all: START_BUILD $(EXECUTABLE)

START_BUILD:
	@echo "start to build project"
	@echo "platform:" $(platform)
	@echo $(HD_VERSION)
	@echo $(OS_VERSION)
	@echo $(CURR_SOFT_VERSION)
	@echo $(ORIGINAL_SOFT_VERSION)
	@echo $(SOFT_UPDATE)
	@echo $(VERSION_NUM)
	@echo $(CHMOD_VERSION)

$(EXECUTABLE):$(OBJS)
	$(CC) -o $(EXECUTABLE) $(OBJS) $(DYNAMIC_LIBS)

${OUTPUT_OBJ_DIR}/%.o : ./main/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./dev/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./inifile/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./log/%.c
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	
${OUTPUT_OBJ_DIR}/%.o : ./log/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./msg/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./network/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./service/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./xml/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./public/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./db/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./thirdparty/xml/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

${OUTPUT_OBJ_DIR}/%.o : ./thirdparty/inifile/%.cpp
	@mkdir -p ${OUTPUT_OBJ_DIR}
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@	

clean :
	$(RM) $(EXECUTABLE) $(EXECUTABLE).st
	$(RM) $(OBJS) 
outPrint :
	@echo ""
	@echo "==========================================================="
	@echo "Output path : ${OUTPUT_DIR}"
	@echo "==========================================================="
	@echo ""
	@echo ""
	@echo ""
