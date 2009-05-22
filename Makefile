all: login.cgi client server

login.cgi: login_cgi.c cgi_util.c cgi_util.h
	gcc login_cgi.c cgi_util.c -o login.cgi

server: server.c
	gcc server.c -o server

client: client.c
	gcc client.c -o client
