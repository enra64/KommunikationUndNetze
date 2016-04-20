# Makefile for echoapp

ARCH64=''
ifeq (64,$(findstring 64,$(shell uname -m)))
	ARCH64=64
endif

CC	= gcc
CFLAGS  += -Wall -g -Iinclude
LDFLAGS += -Llib
LDLIBS  += -lcnaiapi$(ARCH64) -lpthread

APPS = echoclient echoserver

apps: $(APPS)


.PHONY: clean
clean:
	rm -rf *.o $(APPS)
