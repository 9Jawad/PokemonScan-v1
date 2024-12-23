FLAGS=-std=gnu11 -Wall -Wextra -O2 -Wpedantic -g

all: img-search

img-search: img-search.c
	gcc $(FLAGS) img-search.c -o img-search