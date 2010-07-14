#CFLAGS=-g -DDEBUG -Wall
CFLAGS=-O2 -Wall
#CFLAGS=-g -Wall

all: passwdserver login.cgi logout.cgi client benchmark

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

clean:
	rm passwdserver login.cgi logout.cgi client benchmark
