all: input

input : input.c
	gcc -Wall -O0 -g3 input.c -o input
