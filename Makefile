all: passwdserver

#login.cgi: login_cgi.c cgiutil.c cgiutil.h comserial.c comserial.h
	#gcc login_cgi.c cgiutil.c comserial.c -o login.cgi

passwdserver: passwdserver.c comserver.c comserver.h comserver.h
	gcc passwdserver.c comserver.c -o passwdserver
