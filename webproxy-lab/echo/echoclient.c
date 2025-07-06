// 직접 링킹해서 컴파일 하기, 못읽어옴, gcc -o echoclient echoservice.c csapp.c
/*
CC = gcc
CFLAGS = -Wall -O2

all: echoservice echoclient

echoservice: echoservice.c csapp.c
	$(CC) $(CFLAGS) -o $@ $^

echoclient: echoclient.c csapp.c  
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f echoservice echoclient

Makefile 사용시 추가
*/
#include "../csapp.h"
// #define _POSIX_C_SOURCE 200809L

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3){
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL){
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }
    Close(clientfd);
    exit(0);
}