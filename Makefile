all: a.out
	gcc -g -DUSEREADLINE -lreadline -ltermcap -lpthread socks.c -o nutcat
noreadline:
	gcc -g -lpthread socks.c -o nutcat
pedantic:
	gcc -g -lpthread --ansi --pedantic -Werror socks.c -o nutcat
help:
	@echo "make [noreadline]"
clean:
	rm nutcat
a.out: a.c
	gcc -g -lpthread a.c -o a.out
