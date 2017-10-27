#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <pthread.h>

#include <unistd.h>

#define BUFFERLENGTH 2048

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
char buffer[BUFFERLENGTH];

typedef struct {
 char *ext;
 char *mediatype;
} extn;

//possible media types
extn media_types[] = {
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
 	{"rar","application/octet-stream"}
 };

//generic error handler
void error(char *message) {
	fprintf(stderr, "%s\n", message);
	exit(1);
}

//parses request, finds file, and returns result to client within thread
void *http_handler_p(void *socket) {

	char *request = malloc(sizeof(char*));
	char *file = malloc(sizeof(char*)); 
	char *version = malloc(sizeof(char*)); 
	char *message;
	char *status = malloc(sizeof(char*));
	char *file_extension = malloc(sizeof(char*));
	//char *media = malloc(sizeof(char*));
	char c;
	int client_socket = *(int*)socket;
	int length_of_file;
	FILE *fp;
	
	//clear out and fill the buffer
	memset(&buffer, '\0', sizeof(buffer));
	ssize_t length = read(client_socket, buffer, BUFFERLENGTH);
	message = malloc(length);
    strcpy(message, buffer);

    //parse request
    strcpy(request, strtok(message, " "));
    strcpy(file, strtok(NULL, " "));
    strcpy(version, strtok(NULL, "\r"));

    free(message);

    request[strlen(request)] = '\0';
	file[strlen(file)] = '\0';
	version[strlen(version)] = '\0';

	if (strcmp(request, "GET") != 0)
		error("Methods other than GET not implemented.");

	free(request);

    if (strcmp(version, "HTTP/1.1") != 0)
    	error("Invalid HTTP version.");

    free(version);

    //check the file exists and deal with /
    if (strcmp(file, "/") == 0)
    	strcpy(file, "index.html");

    //remove first char if /
    if (file[0] == '/')
    	++file;

    //check if file exists or not
    if (access(file, F_OK) == -1) {
    	strcpy(status, "HTTP/1.1 404 Not Found\r\n");
    	strcpy(file, "404.html");
    } else
    	strcpy(status, "HTTP/1.1 200 OK\r\n");


    //determine file extension
    strcpy(file_extension, file);
    strtok(file_extension,".");
    strcpy(file_extension, strtok(NULL, ""));
    file_extension[strlen(file_extension)] = '\0';
    

    // //determine media type
    // for (int i = 0; i < 15; i++) {
    // 	if (strcmp(media_types[i].ext,file_extension) == 0)
    // 		strcpy(media, media_types[i].mediatype);
    // }
	

    //write headers
    //status code
    write(client_socket, status, strlen(status));
    free(status);

    //content-type
    write(client_socket, "Content-Type: ", 14);
    write(client_socket, file_extension, strlen(file_extension));
    write(client_socket, "\r\n", 2);
    free(file_extension);

    //content-length
    struct stat st;
    stat(file, &st);
    length_of_file = st.st_size;
    char *content_length = malloc(sizeof(char*));
    sprintf(content_length,"%d",length_of_file);
	write(client_socket, "Content-Length: ", 16);
	write(client_socket, content_length, strlen(content_length));
	free(content_length);
	write(client_socket, "\r\n\r\n", 4);

	//lock and open file
	printf("%s\n", file);
	pthread_mutex_lock(&lock);
	fp = fopen(file, "r");
	if (fp == NULL) {
		error("File does not exist.");
	}
	char *buf;
	buf = (char*)calloc(length_of_file, sizeof(char));	
	 
	/* memory error */
	if (buf == NULL)
	    error("Failed to calloc.");
	 
	/* copy all the text into the buffer */
	fread(buf, sizeof(char), length_of_file, fp);
	int position = 0;
	while (buf[position] != EOF) {
		write(client_socket, &buf[position], 1);
		++position;
	}
	free(buf);
	free(file);

	//close file and unlock 
	fclose(fp);
	pthread_mutex_unlock(&lock);

	//tidy up
	char *end = "\r\n\r\n";
	write(client_socket, end, 4);
	
	//free(media);
	printf("done\n");
	return 0;
}

int main(int argc, char **argv) {

	struct sockaddr_in server_address, client_address;
	int port, server_socket, client_socket;
	socklen_t client_length;

	//argument length check
	if (argc < 2)
		error("Port not provided.");

	//port validity check
	port = atoi(argv[1]);
	if (port < 1024 || port > 65535)
		error("Illegal port number.");

	//create socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
		error("Socket failed to open.");

	//clear out server_address
	memset(&server_address, '\0', sizeof(server_address));

	//assign server_address attributes
	server_address.sin_family = AF_INET; //address family code
	server_address.sin_addr.s_addr = INADDR_ANY; //IP address of machine
	server_address.sin_port = htons(port); //port number

	//bind to socket
	if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		error("Failed to bind to socket.");

	//listen for connections with queue size of 128
	listen(server_socket, 128);

	while (1) {

		client_length = sizeof(client_address);
		client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_length);
		if (client_socket < 0)
			error("Failed to accept socket.");

		//create thread
		pthread_t thread;
		pthread_create(&thread, NULL, http_handler_p, (void *) &client_socket);
	}
	
	return 0;
}
