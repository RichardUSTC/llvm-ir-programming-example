#########################################################################
######################## Configuration Area #############################
CC = gcc
CXX = g++
OBJDUMP = objdump

INCLUDES +=
CFLAGS += -g $(shell llvm-config --cflags)
CXXFLAGS += -g $(shell llvm-config --cxxflags)
LDFLAGS += $(shell llvm-config --ldflags)
LIBS += $(shell llvm-config --libs)
DUMP_FLAGS = -D

TARGET_OUTPUT_DIR = bin
DUMP_OUTPUT_DIR = ${TARGET_OUTPUT_DIR}
#########################################################################

ifneq (${INCLUDES}, )
	CFLAGS += -I${INCLUDES}
	CXXFLAGS += -I${INCLUDES}
endif


C_SRC = $(wildcard *.c)
C_TARGET = $(patsubst %.c, ${TARGET_OUTPUT_DIR}/%, ${C_SRC})
C_DUMP= $(patsubst %.c, ${DUMP_OUTPUT_DIR}/%.dump, ${C_SRC})
CXX_SRC = $(wildcard *.cpp)
CXX_TARGET = $(patsubst %.cpp, ${TARGET_OUTPUT_DIR}/%, ${CXX_SRC})
CXX_DUMP= $(patsubst %.cpp, ${DUMP_OUTPUT_DIR}/%.dump, ${CXX_SRC})
TARGET = ${C_TARGET} ${CXX_TARGET}
DUMP = ${C_DUMP} ${CXX_DUMP}

all: ${TARGET}

dump: ${DUMP} ${TARGET}

clean:
	@rm -f ${TARGET} ${DUMP}

${C_TARGET}: ${TARGET_OUTPUT_DIR}/%: %.c
	@mkdir -p `dirname $@`
	${CC} -o $@ $< ${CFLAGS} ${LIBS} ${LDFLAGS}

${CXX_TARGET}: ${TARGET_OUTPUT_DIR}/%: %.cpp
	@mkdir -p `dirname $@`
	${CXX} -o $@ $< ${CXXFLAGS} ${LIBS} ${LDFLAGS}

${DUMP}: ${DUMP_OUTPUT_DIR}/%.dump: ${TARGET_OUTPUT_DIR}/%
	@mkdir -p `dirname $@`
	${OBJDUMP} ${DUMP_FLAGS} $< > $@
