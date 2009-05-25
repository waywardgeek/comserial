all: passwdserver login.cgi

login.cgi: login_cgi.c cgiutil.c cgiutil.h comclient.c comclient.h
	gcc login_cgi.c cgiutil.c comclient.c -o login.cgi

passwdserver: passwdserver.c comserver.c comserver.h comclient.h
	gcc -g passwdserver.c comserver.c -o passwdserver
