all: example.cgi

example.cgi: example_cgi.c cgi_util.c cgi_util.h
	gcc example_cgi.c cgi_util.c -o example.cgi


