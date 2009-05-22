all: login.cgi

login.cgi: login_cgi.c cgiutil.c cgiutil.h
	gcc login_cgi.c cgiutil.c -o login.cgi

#server: server.c
	#gcc server.c -o server

#client: client.c
	#gcc client.c -o client
