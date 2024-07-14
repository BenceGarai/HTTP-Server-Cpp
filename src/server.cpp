#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <fstream>

std::string extractHeader(const std::string& request, const std::string& header_name);
std::string extractMethod(const std::string& request);
std::string extractPath(const std::string& request);

void handleClient(int client_fd, const std::string& dir) {
    std::string ok_message = "HTTP/1.1 200 OK\r\n";
    std::string error_message = "HTTP/1.1 404 Not Found\r\n";

    char buffer[1024];
    int received_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    buffer[received_bytes] = '\0';
    std::cout << "Received from client: " << buffer << "\n";

    // Request
    std::string request(buffer);

    // Extracting data
    std::string method = extractMethod(request);
    std::string path = extractPath(request);
    std::cout << "Path found: " << path << "\n";
    std::string contentType = "Content-Type: text/plain\r\n";

    if (method == "GET" && path == "/") {
        // Default message
        std::string response = ok_message + contentType + "\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (method == "GET" && path.rfind("/echo/", 0) == 0) {
        // Echo response
        std::string echo_message = path.substr(6);
        int conLen = echo_message.length();
        std::string response_message = ok_message + contentType + "Content-Length: " + std::to_string(conLen) + "\r\n\r\n" + echo_message;
        std::cout << "Message sent: " << response_message << "\n";
        send(client_fd, response_message.c_str(), response_message.size(), 0);
    }
    else if (method == "GET" && path == "/user-agent") {
        // User-Agent response
        std::string user_agent = extractHeader(request, "User-Agent: ");
        if (!user_agent.empty()) {
            int user_length = user_agent.length();
            std::string user_agent_response = ok_message + contentType + "Content-Length: " + std::to_string(user_length) + "\r\n\r\n" + user_agent;
            send(client_fd, user_agent_response.c_str(), user_agent_response.size(), 0);
        }
        else {
            // If User-Agent header not found, send an error response
            std::string error_response = error_message + contentType + "\r\n";
            send(client_fd, error_response.c_str(), error_response.size(), 0);
        }
    }
    else if (method == "GET" && path.rfind("/files/", 0) == 0) {
        // File response
        std::string filename = dir + path.substr(7); // Extract the filename
        std::ifstream file(filename, std::ios::binary);
        std::cout << "Here is the filename: " << filename << "\n";

        if (file) {
            file.seekg(0, std::ios::end);
            std::streamsize file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::string response_message = ok_message + "Content-Type: application/octet-stream\r\n" + "Content-Length: " + std::to_string(file_size) + "\r\n\r\n";
            send(client_fd, response_message.c_str(), response_message.size(), 0);

            char file_buffer[1024];
            while (file.read(file_buffer, sizeof(file_buffer))) {
                send(client_fd, file_buffer, sizeof(file_buffer), 0);
            }
            send(client_fd, file_buffer, file.gcount(), 0);
        }
        else {
            std::string error_response = error_message + contentType + "\r\n";
            send(client_fd, error_response.c_str(), error_response.size(), 0);
        }
    }
    else {
        // Error response
        std::string error_response = error_message + contentType + "\r\n";
        send(client_fd, error_response.c_str(), error_response.size(), 0);
    }

    close(client_fd);
}

int main(int argc, char** argv) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string dir;

    if (argc > 2 && std::string(argv[1]) == "--directory") {
        dir = argv[2];
        std::cout << "Accepted argument: " << dir << "\n";
    }

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
        // Create and detach thread
        std::thread(handleClient, client_fd, dir).detach();
    }

    close(server_fd);
    return 0;
}

std::string extractHeader(const std::string& request, const std::string& header_name) {
    size_t header_start = request.find(header_name);
    if (header_start != std::string::npos) {
        size_t value_start = header_start + header_name.length();
        size_t value_end = request.find("\r\n", value_start);
        if (value_end != std::string::npos) {
            return request.substr(value_start, value_end - value_start);
        }
    }
    return "";
}

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
            return request.substr(path_start, path_end - path_start);
        }
    }
    return "";
}

