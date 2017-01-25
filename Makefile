all: PMan
	gcc -g  -o PMan PMan.c

clean:
	-rm -f PMan

run: PMan
	./PMan
