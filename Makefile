CFLAGS=-g -DDEBUG

all: passwdserver login.cgi testclient

login.cgi: login_cgi.c cgiutil.c cgiutil.h comclient.c comclient.h
	gcc $(CFLAGS) login_cgi.c cgiutil.c comclient.c -o login.cgi

passwdserver: passwdserver.c comserver.c comserver.h comclient.h
	gcc $(CFLAGS) passwdserver.c comserver.c -o passwdserver

testclient: testclient.c comclient.c comclient.h
	gcc $(CFLAGS) testclient.c comclient.c -o testclient

