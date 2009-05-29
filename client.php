#!/usr/bin/php
<?php
session_start();

# Read from the socket until we read a '\0'
function readResponse($sock)
{
    $response = "";
    $c = socket_read($sock, 1, PHP_BINARY_READ);
    while(ord($c) != 0) {
	$response .= $c;
	$c = socket_read($sock, 1, PHP_BINARY_READ);
    }
    return $response;
}

$sock = socket_create(AF_UNIX, SOCK_STREAM, 0);
socket_connect($sock, "/tmp/test");
socket_write($sock, $PHPSESSID + chr(0));
print(readResponse($sock));
do {
    $line = rtrim(fgets(STDIN));
    if(strcmp($line, "quit")) {
	socket_write($sock, $line . chr(0));
	print(readResponse($sock));
    }
} while(strcmp($line, "quit"));
?>
