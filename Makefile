all: a.out

help:
	@echo "make [clean]"
clean:
	rm -rf a.out
a.out: a.c
	gcc -g -lpthread a.c -o a.out
