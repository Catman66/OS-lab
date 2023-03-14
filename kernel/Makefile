export TOKEN := KYfFcnKg

NAME := kernel
SRCS := kernel.c
export MODULE  := L0						
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
