#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <unistd.h>

#define SOCKET_READ_BUFFER 4096


int main() {
    /* create a socket to connect to the server */
    int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    /* allow socket re-use */
    // setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(NULL));

    /* resolve the hostname of the server */
    struct hostent *host_entity;
    if ((host_entity = gethostbyname("localhost")) == NULL) {
        perror("Could not resolve hostname");              
        exit(EXIT_FAILURE);
    }

    /* configure the socket for communication with the server */
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    memcpy(&server_address.sin_addr, host_entity->h_addr_list[0], host_entity->h_length);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(1337);

    /* connect to the server */
    int connection_result = connect(
        socket_file_descriptor,
        (struct sockaddr *) &server_address,
        sizeof(server_address)
    );
    if (connection_result == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    /* Write a message to the server */
    char *message = "Hello, world! This is a long message made up of many characters :-)";
    int write_result = send(socket_file_descriptor, message, strlen(message), 0);
    if (write_result == -1) {
        perror("Failed to write to the server");
        exit(EXIT_FAILURE);
    }

    /* 
     * Indicate that we do not wish to send any more data, but still wish to
     * receive bytes back over the connection. This keeps the server from
     * hanging on our connection
     */
    int close_write_connection = shutdown(socket_file_descriptor, SHUT_WR);
    if (close_write_connection == -1) {
        perror("Error closing write connection");
        exit(EXIT_FAILURE);
    }

    /* buffered reading of the connection contents */
    char buffer[SOCKET_READ_BUFFER];
    do {
        /* clear the buffer so as not to get garbled data */
        memset(buffer, 0, SOCKET_READ_BUFFER);

        /* read the response from the server*/
        int bytes_read = recv(socket_file_descriptor, buffer, SOCKET_READ_BUFFER, 0);
        
        /* when recv returns zero, we are done receiving data*/
        if (bytes_read == 0) {
            break;
        }

        /* error handling */
        if (bytes_read == -1) {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }

        /* stream server data to the console so we don't need to bother storing it */
        printf("%.*s", SOCKET_READ_BUFFER, buffer);
    } while (1);

    /* we're done with the connection and can close it*/
    close(socket_file_descriptor);

    /* clean up and exit */
    printf("\n");
    exit(EXIT_SUCCESS);
}