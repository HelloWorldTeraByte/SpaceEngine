CC=gcc
CFLAGS=-Wall -g 
LINK=-lSDL2 -lSDL2_image -lm -lSDL2_ttf

main: main.c
	$(CC) $(CFLAGS) main.c -o build/exec $(LINK) 
clean:
	rm -f build/exec

rebuild: clean main

