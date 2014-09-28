all: a.out

help:
	@echo "make [clean]"
clean:
	rm -rf a.out
a.out: proto1.c
	gcc -g -lpthread proto1.c -o a.out
