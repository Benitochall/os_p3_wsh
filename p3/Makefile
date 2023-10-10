CC = gcc
CFLAGS = -Wall -Werror -pedantic -std=gnu18
LOGIN = challifour
SUBMITPATH = /home/cs537-1/handin/challifour/P3

all:
	wsh
wsh: wsh.c wsh.h
	$(CC) $< $(CFLAGS) -o $@

run: wsh
	./$^

pack: wsh.h wsh.c Makefile README.md
	tar -czvf login.tar.gz $^

submit: pack
	cp login.tar.gz $(SUBMITPATH)


