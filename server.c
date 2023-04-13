#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "/usr/lib/llhttp-main/build/llhttp.h"
/**
 * @brief The IP address to bind to for incoming connections. 
 */
#define BIND_ADDR "127.0.0.1"

/**
 * @brief The port on which clients should connect.
 */
#define BIND_PORT 7070

/**
 * @brief The I/O buffer size of the server in bytes.
 */
#define BUFFER_SIZE 2048


/**
 * @brief Callback function that is invoked when an the URL is received.
 * 
 * This function is called when the parser encounters the URL in an HTTP 
 * request message
 * 
 * @return the URL of the request.
 */
int on_url(llhttp_t* parser, const char* at, size_t length) {
    
char* url_buffer = (char*)malloc(length + 1);

memcpy(url_buffer, at, length);
url_buffer[length] = '\0';
/* Store the URL in the user data field of the parser */
parser->data = url_buffer;   
//free(url_buffer);
return 0;
}


/**
 * @brief Main entrypoint of the test HTTP server.
 * 
 * This function starts a web server which listens for incoming connections and
 * serves a file from the filesystem.
 * 
 * @return EXIT_SUCCESS when everything worked.
 */
int main(void)
{
    char buffer[BUFFER_SIZE] = {};

    llhttp_t parser;
    llhttp_settings_t settings;
    llhttp_settings_init(&settings);
    llhttp_init(&parser,HTTP_BOTH , &settings);


    settings.on_url = on_url;


    /* Create the server socket used to listen for incoming connections */
    int s_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(s_socket < 0){
        fprintf(stderr, "Could not create server socket (%d): %s\n", errno,
            strerror(errno));
        return EXIT_FAILURE;
    }
    
    /* Bind the server socket to the given port and ip */
    struct sockaddr_in s_addr = {};
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(BIND_PORT);
    s_addr.sin_addr.s_addr = inet_addr(BIND_ADDR);
    
    if(bind(s_socket,(struct sockaddr*) &s_addr, sizeof(s_addr)) < 0) {
        fprintf(stderr, "Could not bind server socket (%d): %s\n", errno,
            strerror(errno));
        return EXIT_FAILURE;
    }
    
    /* Start listening for incoming connections */
    if(listen(s_socket, 8) < 0){
        fprintf(stderr, "Could not start listening (%d): %s\n", errno,
            strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Started listening for HTTP clients on %s:%d\n", BIND_ADDR,
        BIND_PORT);

    while(1) {
        /* Accept an incoming connection */
        struct sockaddr_in c_addr = {};
        socklen_t c_size = sizeof(c_addr);
        int c_socket = accept(s_socket, (struct sockaddr*) &c_addr, &c_size);
        
        if(c_socket < 0){
            fprintf(stderr, "Could not accept incoming client connection "
                "(%d): %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        printf("Client connected at IP: %s and port: %i\n",
            inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
        
        long req_len = read(c_socket, buffer, BUFFER_SIZE);
        printf("%.*s\n", (int) req_len, buffer);
        
        llhttp_execute(&parser, buffer, req_len);

        
                       

        /* Store the URL of the request*/
        char* url = (char*)parser.data;
        char str1[] = ".";
        char*  str2 = url;
        char * targetURL;
        targetURL = (char *) malloc(strlen(str1) + strlen(str2) + 1);
        strcpy(targetURL, str1);
        strcat(targetURL, str2);




        /* Open the file */
        FILE *fptr;
        fptr = fopen(targetURL, "r");
   
        /* Find the size of the file */   
        fseek(fptr, 0, SEEK_END);
        int file_size = ftell(fptr); 

        /* Store the content of the file in bodyContent */
        rewind(fptr);
        char*  bodyContent = (char*)malloc(sizeof(char) * (file_size + 1));    
        fread(bodyContent, sizeof(char),file_size, fptr);
    
        /* Close the file */
        fclose(fptr);

        /* Create the header */
        char header[2000];
        strcpy(header, "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ");
        char size[10];
        sprintf(size, "%d", file_size);
        char * inter = strcat (header,size);
        char * inter2 = strcat(inter,"\n\n");
  
        /* Create the message */
        char * message = strcat( inter,bodyContent);


        //size_t header_len = snprintf(buffer, 512,
           // "HTTP/1.1 200 OK\n"
           // "Content-Type: text/html\n"
            //"Content-Length: 20\n\n");
        
        //if(send(c_socket, buffer, header_len, 0) < 0){
          //  fprintf(stderr, "Could not send response headers (%d): %s\n", errno,
            //    strerror(errno));
            //return EXIT_FAILURE;
       // }

        if(send(c_socket, message, strlen(message), 0) < 0){
            fprintf(stderr, "Could not send response (%d): %s\n", errno,
                strerror(errno));
            return EXIT_FAILURE;
        }

    close(c_socket);
      
    }
        
    close(s_socket);
    return EXIT_SUCCESS;
}