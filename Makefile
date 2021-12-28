sand: src/*.c
	cc -Wall -Wextra -Wshadow -Werror -std=c11 -pedantic $(shell pkg-config raylib --cflags) $(shell pkg-config raylib --libs) src/*.c -g -o sand
