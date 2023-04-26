export TOKEN := KYfFcnKg

export MODULE  := L2
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

test: git time-record
	@gcc $(shell find src/ -name "*.c")  \
			$(shell find test/ -name "*.c") \
			-Iframework -Itest -DTEST -lpthread \
			-o build/test -g
	@echo running &&\
	 /usr/bin/time -o time-record -p ./build/test &&\
	 echo end &&\
	cat time-record | grep real | cut -f2 -d' ' > real.time &&\
	cat time-record | grep sys | cut -f2 -d' '> sys.time &&\
	cat time-record | grep user | cut -f2 -d' ' > user.time &&\
	paste *.time >> ttime &&\
	tail -5 ttime

	


