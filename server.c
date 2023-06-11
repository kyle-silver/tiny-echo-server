#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* 
 * this buffer is way too small, but it helps catch issues that only occur at 
 * buffer boundaries
 */
#define SOCKET_READ_BUFFER 30

int handle_connection(int client_file_descriptor);

int main() {
    int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    /* allow for connection re-use*/
    setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(NULL));

    /* set up server address struct */
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(1337);

    /* 
     * bind to the port: associate the above configuration with the file
     * descriptor for the socket we opened
     */
    int bind_result = bind(
        socket_file_descriptor,
        (struct sockaddr *) &server_address,
        sizeof(server_address)
    );
    if (bind_result == -1) {
        perror("Could not assign name to the socket");
        exit(EXIT_FAILURE);
    }

    /* listen on the bound socket */
    if (listen(socket_file_descriptor, 50) == -1) {
        perror("Failure listening to socket");
        exit(EXIT_FAILURE);
    }

    int connection_handling_result = 0;
    do {
        /* accept an incoming connection */
        struct sockaddr_in peer_address;
        socklen_t peer_address_size = sizeof(peer_address);
        int client_file_descriptor = accept(
            socket_file_descriptor,
            (struct sockaddr *) &peer_address,
            &peer_address_size
        );
        if (client_file_descriptor == -1) {
            perror("Could not accept client connection");
            exit(EXIT_FAILURE);
        }

        /* hand off processing to our helper function */
        connection_handling_result = handle_connection(client_file_descriptor);

        /* clean up the connection */
        close(client_file_descriptor);
        printf("Connection closed: %d\n", client_file_descriptor);
    } while (connection_handling_result == 0);

    /* crash the whole server on any error */
    perror("Connection mishandled");
    exit(EXIT_FAILURE);
}

int handle_connection(int client_file_descriptor) {
    /* create a buffer to read the contents of the client request */
    char buffer[SOCKET_READ_BUFFER];

    do {
        /* receive data from the client */
        memset(buffer, 0, SOCKET_READ_BUFFER);
        int bytes_read = recv(client_file_descriptor, buffer, SOCKET_READ_BUFFER, 0);
        
        /* recv returns 0 when there is no more data to read*/
        if (bytes_read == 0) {
            return 0;
        }

        /* a negative number indicates an error */
        if (bytes_read == -1) {
            perror("Error reading from socket");
            return -1;
        }

        /* print the first page of contents */
        printf("Client request (%d): %.*s\n",client_file_descriptor, bytes_read, buffer);

        /* 
         * stream the received data back to the client so we don't need to 
         * hold more than a single buffer's worth of data in memory per
         * connection.
         */
        int bytes_written = send(client_file_descriptor, buffer, bytes_read, 0);
        if (bytes_written == -1) {
            perror("Error writing to client socket");
            return -1;
        }
    } while (1);
}