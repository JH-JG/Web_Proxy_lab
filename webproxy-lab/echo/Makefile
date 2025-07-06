CC = gcc
CFLAGS = -Wall -O2

CSAPP = ../csapp.c

all: echoservice echoclient

echoservice: echoservice.c $(CSAPP)
	$(CC) $(CFLAGS) -o $@ $^

echoclient: echoclient.c $(CSAPP) 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f echoservice echoclient