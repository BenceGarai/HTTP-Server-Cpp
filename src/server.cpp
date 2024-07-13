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

void handleClient(int client_fd) {
    std::string ok_message = "HTTP/1.1 200 OK\r\n";
    std::string error_message = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
    char buffer[1024];

    int received_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    buffer[received_bytes] = '\0';
    std::cout << "Received from client: " << buffer << "\n";

    std::string request(buffer);
    std::string method = extractMethod(request);
    std::cout << "Method extracted: " << method << "\n";
    std::string path = extractPath(request);
    std::cout << "Path extracted: " << path << "\n";
    std::string contentType = "Content-Type: text/plain\r\n";
    std::cout << "ContentType extracted: " << contentType << "\n";

    if (method == "GET" && path == "/") {
        std::string response = ok_message + contentType + "\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (method == "GET" && path.rfind("/echo/", 0) == 0) {
        int conLen = path.substr(6).length();
        std::string echo_message = ok_message + contentType + "Content-Length: " + std::to_string(conLen) + "\r\n\r\n" + path.substr(6);
        std::cout << "Message sent: " << echo_message << "\n";
        send(client_fd, echo_message.c_str(), echo_message.size(), 0);
    }
    else {
        send(client_fd, error_message.c_str(), error_message.size(), 0);
    }

    close(client_fd);
}

int main(int argc, char** argv) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    char buffer[1024];

    std::cout << "Logs from your program will appear here!\n";

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    int server_addr_len = sizeof(server_addr);

    if (bind(server_fd, (struct sockaddr*)&server_addr, server_addr_len) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    while (true) {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
        if (client_fd < 0) {
            std::cout << "Failed to accept client.\n";
            continue;
        }
        std::cout << "Client connected\n";
        handleClient(client_fd);
    }

    close(server_fd);
    return 0;
}
