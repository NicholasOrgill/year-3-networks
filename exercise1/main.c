#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char **argv) {
    struct sockaddr_in addr, client_addr;
    int port;
    char buffer[1024];
    char *space = " ";
    char *request[3];
    char *ok = "HTTP/1.1 200 OK\r\n";
    char *nf = "HTTP/1.1 404 Not Found\r\n";

    //arg length check
    if (!argv[1]) {
        fprintf(stderr, "No port included.\n");
        return 1;
    }

    //get port from argv
    port = atoi(argv[1]);
    printf("Connecting to port %d\n", port);

    //create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //checks socket is working
    if (sock < 0) {
        fprintf(stderr, "Failed to create socket.\n");
        return 1;
    }

    //basic stuff
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    //bind error check
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Failed to bind to socket.\n");
        return 1;
    }
    //start listening
    listen(sock, 5);

    //loop until ctrl-c is pressed
    while (1) {

        //obtain size of client_addr
        socklen_t client_size = sizeof(client_addr);

        //perform accept on client socket
        int client_socket = accept(sock, (struct sockaddr *) &client_addr, &client_size);

        //reads in the request
        ssize_t length = read(client_socket, buffer, 1024);
        char *req = malloc(sizeof(char *));
        strcpy(req, buffer);

        //sets each part of the http request to a separate char* in the char** request
        for (int i = 0; i < 3; i++) {
            if (i == 0) request[i] = strtok(req, space);
            else request[i] = strtok(NULL, space);
        }

        //checks request is GET
        if (strcmp(request[0], "GET") != 0) {
            fprintf(stderr, "Not handling a GET request.\n");
            return 1;
        }

        //allocate space for file extension for checking
        char *filename = malloc(sizeof(char *));
        strcpy(filename, request[1]);

        //just to eliminate the file name
        strtok(filename, ".");

        //store the file extension
        char *extension = malloc(sizeof(char *));
        if (strcmp(filename, "/") == 0) {
            //set as index
            strcpy(extension, "html");
        } else extension = strtok(NULL, ".");
        printf("%s\n", extension);

        //check what type the file is
        char *content_type;
        if (strcmp(extension, "html") == 0) {
            content_type = "Content Type: text/html\r\n\r\n";
        } else if (strcmp(extension, "css") == 0) {
            content_type = "Content Type: text/css\r\n\r\n";
        } else if ((strcmp(extension, "jpeg") == 0) || strcmp(extension, "jpg") == 0) {
            content_type = "Content Type: img/jpeg\r\n\r\n";
        } else {
            content_type = "Content Type: img/gif\r\n\r\n";
        }

        //opens file and checks if it worked
        FILE *fp;
        if (strcmp(request[1], "/") == 0) {
            fp = fopen("index.html", "r");
        } else {
            request[1]++;
            fp = fopen(request[1], "r");
        }
        if (!fp) {

            //file did not open correctly, inform client and close
            fprintf(stderr, "Failed to read file.\n");
            send(client_socket, nf, strlen(nf), 0);
            close(client_socket);
            return 1;
        }

        //everything is fine
        send(client_socket, ok, strlen(ok), 0);
        send(client_socket, content_type, strlen(content_type), 0);

        //reads file into buffer and send to client until whole file has been sent
        while (fread(buffer, 1024, 1, fp)) {
            send(client_socket, buffer, 1024, 0);
        }
        close(client_socket);
    }
    return 0;
}