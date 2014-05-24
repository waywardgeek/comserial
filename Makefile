#CFLAGS=-g -DDEBUG -Wall
CFLAGS=-O2 -Wall
#CFLAGS=-g -Wall

all: passwdserver login.cgi logout.cgi client benchmark libcomserver.a libcomclient.a

login.cgi: login_cgi.c cgiutil.c cgiutil.h comclient.c comclient.h
	gcc $(CFLAGS) login_cgi.c cgiutil.c comclient.c -o login.cgi

logout.cgi: logout_cgi.c cgiutil.c cgiutil.h comclient.c comclient.h
	gcc $(CFLAGS) logout_cgi.c cgiutil.c comclient.c -o logout.cgi

passwdserver: passwdserver.c comserver.c comserver.h comclient.h
	gcc $(CFLAGS) passwdserver.c comserver.c -o passwdserver

client: client.c comclient.c comclient.h cgiutil.c cgiutil.h
	gcc $(CFLAGS) client.c comclient.c cgiutil.c -o client

benchmark: benchmark.c comclient.c comclient.h cgiutil.c cgiutil.h
	gcc $(CFLAGS) benchmark.c comclient.c cgiutil.c -o benchmark

libcomserver.a: comserver.o
	ar rcs libcomserver.a comserver.o

comserver.o: comserver.c comserver.h comclient.h
	gcc $(CFLAGS) -c comserver.c

libcomclient.a: comclient.o cgiutil.o
	ar rcs libcomclient.a comclient.o cgiutil.o

comclient.o: comclient.c comclient.h cgiutil.h
	gcc $(CFLAGS) -c comclient.c

cgiutil.o: cgiutil.c cgiutil.h
	gcc $(CFLAGS) -c cgiutil.c

clean:
	rm passwdserver login.cgi logout.cgi client benchmark *.a *.o

install:
	cp comserver.h /usr/include
	cp libcomserver.a libcomclient.a /usr/lib
