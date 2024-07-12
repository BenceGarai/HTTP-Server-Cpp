#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char **argv) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    char buffer[1024];

    std::string OK_MESSAGE = "HTTP/1.1 200 OK\r\n\r\n";

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here!\n";

    // Create socket which is assigned to an integer as a file descriptor fd
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    // sockaddr_in is used to store socket related information used for ipv4 addresses
    struct sockaddr_in server_addr;
    // Specifies address family in this case : AF_INET which is Ipv4
    server_addr.sin_family = AF_INET;
    // sin_addr.s_addr hold the IP address for the socket
    // INADDR_ANY allows the server to accept connections from any IP
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // Assigning port number for incoming connections
    server_addr.sin_port = htons(4221);

    int server_addr_len = sizeof(server_addr);

    // Assossciate socket with the specific addresss (server_addr) above
    if (bind(server_fd, (struct sockaddr*)&server_addr, server_addr_len) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    // Setting limit of pending connections to 5
    int connection_backlog = 5;
    // Listen marks the socket as passive to accept incoming connections
    // returns 0 in success
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    // Store details of the client
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    // Accept incoming connections on the socket (server_fd)
    // Insert client data into client_addr and client_addr_len
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
    if (client_fd < 0) {
        std::cout << "Failed to accept client.";
            return -1;
    }
    std::cout << "Client connected\n";

    send(client_fd, OK_MESSAGE.c_str(), OK_MESSAGE.size(), 0);

    close(client_fd);
    close(server_fd);

    return 0;
}
