#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> 
#include <ws2tcpip.h> 
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <pthread.h> 


int total_bytes_sent = 0;// voor het bijhouden van gestuurde bytes.

void OSInit(void)
{
    WSADATA wsaData;
    int WSAError = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (WSAError != 0)
    {
        fprintf(stderr, "WSAStartup errno = %d\n", WSAError);
        exit(-1);
    }
}
#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
void OSInit(void) {}
void OSCleanup(void) {}
#endif

int initialization();
int connection(int internet_socket);
void execution(int internet_socket);
void cleanup(int internet_socket, int client_internet_socket);

int main(int argc, char* argv[])
{
    printf("Program Start\n");
    OSInit();
    int internet_socket = initialization();

    while (1) {
        int client_internet_socket = connection(internet_socket);
        execution(client_internet_socket);
    }


  
    

}

int initialization()
{
    // Step 1.1: Initialize the addrinfo structure
    struct addrinfo internet_address_setup;
    struct addrinfo* internet_address_result;

    // Zero out the internet_address_setup structure
    memset(&internet_address_setup, 0, sizeof(internet_address_setup));

    // Set the address family to unspecified (allow both IPv4 and IPv6)
    internet_address_setup.ai_family = AF_UNSPEC;

    // Set the socket type to stream (TCP)
    internet_address_setup.ai_socktype = SOCK_STREAM;

    // Set the flags to indicate the socket will be used for passive binding
    internet_address_setup.ai_flags = AI_PASSIVE;

    // Get address information for binding the socket to port 22
    int getaddrinfo_return = getaddrinfo(NULL, "22", &internet_address_setup, &internet_address_result);

    // Check if getaddrinfo failed
    if (getaddrinfo_return != 0) {
        // Print an error message and exit the program with an error code
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
        exit(1);
    }

    // Initialize the internet_socket to an invalid value
    int internet_socket = -1;

    // Initialize an iterator for the address result linked list
    struct addrinfo* internet_address_result_iterator = internet_address_result;

    // Iterate through the linked list of address results
    while (internet_address_result_iterator != NULL) {
        // Step 1.2: Create a socket with the current address result
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);

        // Check if the socket creation failed
        if (internet_socket == -1) {
            // Print an error message
            perror("socket");
        }
        else {
            // Step 1.3: Bind the socket to the current address
            int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);

            // Check if the bind call failed
            if (bind_return == -1) {
                // Print an error message and close the socket
                perror("bind");
                close(internet_socket);
            }
            else {
                // Step 1.4: Listen for incoming connections on the bound socket
                int listen_return = listen(internet_socket, SOMAXCONN); // use SOMAXCONN for a system-defined maximum backlog value

                // Check if the listen call failed
                if (listen_return == -1) {
                    // Close the socket and print an error message
                    close(internet_socket);
                    perror("listen");
                }
                else {
                    // Successfully created, bound, and listening on the socket; break the loop
                    break;
                }
            }
        }
        // Move to the next address result in the linked list
        internet_address_result_iterator = internet_address_result_iterator->ai_next;
    }

    // Free the memory allocated for the address results
    freeaddrinfo(internet_address_result);

    // Check if no valid socket was found
    if (internet_socket == -1) {
        // Print an error message and exit the program with an error code
        fprintf(stderr, "socket: no valid socket address found\n");
        exit(2);
    }

    // Return the valid internet socket
    return internet_socket;

}

char ip_address[INET6_ADDRSTRLEN];

int connection(int internet_socket) {
    // Print a message indicating the server is waiting for a connection
    printf("Waiting \n ready to get spammed?\n");

    // Declare a sockaddr_storage structure to hold the client's address information
    struct sockaddr_storage client_internet_address;

    // Declare a variable to store the length of the client's address
    socklen_t client_internet_address_length = sizeof(client_internet_address);

    // Accept a new connection from a client
    int client_socket = accept(internet_socket, (struct sockaddr*)&client_internet_address, &client_internet_address_length);

    // Check if the accept call failed
    if (client_socket == -1) {
        // Print an error message and close the internet socket, then exit the program with an error code
        perror("accept");
        close(internet_socket);
        exit(3);
    }

    // Declare a void pointer to store the address of the client
    void* addr;

    // Check if the client's address is IPv4
    if (client_internet_address.ss_family == AF_INET) {
        // Cast the client address to sockaddr_in and get the IPv4 address
        struct sockaddr_in* s = (struct sockaddr_in*)&client_internet_address;
        addr = &(s->sin_addr);
    }
    // If the client's address is IPv6
    else {
        // Cast the client address to sockaddr_in6 and get the IPv6 address
        struct sockaddr_in6* s = (struct sockaddr_in6*)&client_internet_address;
        addr = &(s->sin6_addr);
    }

    // Declare a character array to store the IP address as a string
    char ip_address[INET6_ADDRSTRLEN];

    // Convert the client's IP address to a string
    inet_ntop(client_internet_address.ss_family, addr, ip_address, sizeof(ip_address));

    // Open the log file in append mode
    FILE* log_file = fopen("log.txt", "a");

    // Check if the log file could not be opened
    if (log_file == NULL) {
        // Print an error message, close the client socket, and exit the program with an error code
        perror("fopen");
        close(client_socket);
        exit(4);
    }

    // Write the connection information to the log file
   
    fprintf(log_file, "Connection from %s\n", ip_address);
   

    // Close the log file
    fclose(log_file);

    // Return the client socket to the caller
    return client_socket;

}



void http_get() {
    int sockfd;
    struct sockaddr_in server_addr;
    char request[256];
    char response[1024];

    // Making a new connection
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    server_addr.sin_addr.s_addr = inet_addr("208.95.112.1");//ip-api

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return;
    }

    // Prepare HTTP request
    snprintf(request, sizeof(request), "GET /json/%s HTTP/1.0\r\nHost: ip-api.com\r\n\r\n", ip_address);

    // Send the HTTP request
    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("send");
        return;
    }

    //opening and writing in file 
    FILE* file = fopen("log.txt", "a");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    while (1) {
        ssize_t bytes_received = recv(sockfd, response, 1024 - 1, 0);
        if (bytes_received == -1) {
            perror("recv");
            break;
        }
        else if (bytes_received == 0) {
            break;
        }

        response[bytes_received] = '\0';

      
        fprintf(file, "Response from GET request:\n%s\n", response);
      
      
        printf("Response from GET request:\n%s\n", response);
     
    }

    fclose(file);

    // Close the connection
    close(sockfd);
}

void* send_message(void* arg) {
    int client_internet_socket = *(int*)arg;
    const char* message = "enjoy\n";
     


    printf("\nStarted spam\n");
    while (1) {
        int bytes_sent = send(client_internet_socket, message, strlen(message), 0);
        if (bytes_sent == -1) {
            perror("send");
            break;
        }
        usleep(100000); // snelheid van het sturen 
        total_bytes_sent += bytes_sent;
    }
    printf("\nFinished Attack\n");

    return NULL;
}


void execution(int client_internet_socket) {
    // Step 1: Receive initial data
    printf("\nExecution Start!\n");
    http_get();
    char buffer[1000];

    // Create a new thread to send message
    pthread_t send_thread;
    pthread_create(&send_thread, NULL, send_message, &client_internet_socket);

    //listening
    while (1) {
        int number_of_bytes_received = recv(client_internet_socket, buffer, sizeof(buffer) - 1, 0);
        if (number_of_bytes_received == -1) {
            perror("recv");
            break;
        }
        else if (number_of_bytes_received == 0) {
            // closed conection or crashed
            printf("Client closed the connection.\n");
            break;
        }

        buffer[number_of_bytes_received] = '\0';
        printf("Received: %s\n", buffer);

        // Write the received message in the log file
        FILE* log_file = fopen("log.txt", "a");
        if (log_file == NULL) {
            perror("fopen");
            break;
        }
        fprintf(log_file, "Message from client: %s\n", buffer);
        fclose(log_file);
    }

   
    pthread_join(send_thread, NULL);

   //aantal geleverde bytes printen 
    FILE* log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        perror("fopen");
        close(client_internet_socket);
        exit(4);
    }
   
    fprintf(log_file, "Total bytes delivered: %d\n", total_bytes_sent);
 
    fclose(log_file);

    printf("Total bytes delivered: %d\n", total_bytes_sent);
 

    // Close the client connection
    close(client_internet_socket);
}