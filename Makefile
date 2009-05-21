all: login.cgi

login.cgi: login_cgi.c cgi_util.c cgi_util.h
	gcc login_cgi.c cgi_util.c -o login.cgi


