#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

std::string extractMethod(const std::string& request) {
    size_t method_end = request.find(" ");
    if (method_end != std::string::npos) {
        return request.substr(0, method_end);
    }
    return "";
}

std::string extractPath(const std::string& request) {
    size_t method_end = request.find(" ");
    if (method_end != std::string::npos) {
        size_t path_start = method_end + 1;
        size_t path_end = request.find(" ", path_start);
        if (path_end != std::string::npos) {
            return request.substr(path_start, path_end - path_start); // Extract and return path
        }
    }
    return ""; // Return empty string if not found
}

int main(int argc, char **argv) {

    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string ok_message = "HTTP/1.1 200 OK\r\n\r\n";
    std::string error_message = "HTTP/1.1 404 Not Found\r\n\r\n";
    char buffer[1024];

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

    int received_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    // \0 makes received_bytes be treated as a string
    // buffer now contains the received  from the client
    buffer[received_bytes] = '\0';
    std::cout << "Received from client: " << buffer << "\n";
    
    std::string request(buffer);

    std::string method = extractMethod(request);
    std::string path = extractPath(request);

    if (method == "GET" && path == "/") {
        send(client_fd, ok_message.c_str(), ok_message.size(), 0);
    }
    else {
        send(client_fd, error_message.c_str(), error_message.size(), 0);
    }
       

    close(client_fd);
    close(server_fd);

    return 0;
}
