CFLAGS = -fsanitize=address -std=c99 -Wall -Werror 

all:
	gcc -g -std=c99 -Wall -fsanitize=address -o alloc alloc.c -lm
clean:
	rm alloc