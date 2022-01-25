UNAME := $(shell uname)

LIBS := $(shell pkg-config raylib --libs)

ifeq ($(UNAME), Linux)
	LIBS += -lGL -lm -lpthread -ldl -lrt -lX11
endif

sand: src/*.c
	cc -Wall -Wextra -Wshadow -Werror -std=c11 $(shell pkg-config raylib --cflags) src/*.c -g -o sand $(LIBS)
