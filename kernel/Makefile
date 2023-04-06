export TOKEN := KYfFcnKg

export MODULE  := L1
NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c")
INC_PATH       := include/ framework/

export AM_HOME := $(PWD)/../abstract-machine

#check if ARCH 已定义
ifeq ($(ARCH),)
export ARCH := x86_64-qemu
endif

#将abstract machine中的makefile文件拷贝进来
include $(AM_HOME)/Makefile

#copy makefile in parent dir here
include ../Makefile.lab
image: git

test: git
	@gcc $(shell find src/ -name "*.c")  \
			$(shell find test/ -name "*.c") \
			-Iframework -Itest -DTEST -lpthread \
			-o build/test
	@build/test 
