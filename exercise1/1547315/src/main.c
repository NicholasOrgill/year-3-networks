#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
	char *key;
	char *value;
} hash_table;

//possible media types
hash_table media_types[] = {
	{"gif", "image/gif"},
	{"txt", "text/plain"},
 	{"jpg", "image/jpg"},
 	{"jpeg","image/jpeg"},
 	{"png", "image/png"},
 	{"ico", "image/ico"},
 	{"zip", "image/zip"},
 	{"gz",  "image/gz"},
 	{"tar", "image/tar"},
 	{"htm", "text/html"},
 	{"html","text/html"},
 	{"pdfhp", "text/html"},
 	{"pdf","application/pdf"},
 	{"zip","application/octet-stream"},
 	{"rar","application/octet-stream"},
 	{0,0}
};

//possible status codes
hash_table status_codes[] = {
	{"200", "200 OK"},
	{"404", "404 Not Found"},
	{"500", "500 Internal Server Error"},
} 

void error(char *message) {
	fprintf(stderr, message);
	exit(1);
}

int main(int argc, char **argv) {

	//defining stuff
	int port = 0, sock;
	socklen_t client_length;
	struct sockaddr_in server_address

	//argument length check
	if (argc < 2) error("Port not provided.\n");

	//port validity check
	port = atoi(argv[1]);
	if (port < 1024 || port > 65535) error("Illegal port number.\n");

	//create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) error("Socket failed to open.\n");

	//clear and set server_address
	memset(&server_address, '\0', sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	//bind to socket
	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) error("Failed to bind to socket.\n");

	//TODO
	
	return 0;
}