#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

int main() {
    // std::cout << "Hello World!" << '\n';

#ifdef _WIN32
    const int LISTEN_PORT = 8080;

    // 初始化 winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    // 创建 socket
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 绑定地址
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTEN_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 监听
    if (listen(server_fd, 5) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port " << LISTEN_PORT << std::endl;

    while (true) {
        // 接受连接
        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        SOCKET client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        // 回声
        char buffer[1024];
        while (true) {
            int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes == 0) {
                std::cout << "Client disconnected" << std::endl;
                break;
            } else if (bytes == SOCKET_ERROR) {
                std::cerr << "Failed to receive data from client: " << WSAGetLastError() << std::endl;
                break;
            }

            buffer[bytes] = '\0';
            std::cout << "Received: " << buffer << std::endl;
            if (send(client_fd, buffer, bytes, 0) == SOCKET_ERROR) {
                std::cerr << "Failed to send data to client: " << WSAGetLastError() << std::endl;
                break;
            }
        }

        closesocket(client_fd);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
#else
    std::cout << "THIS PROGRAM IS NOT SUPPORTED ON THIS PLATFORM CURRENTLY" << std::endl;

    return 1;
#endif

    return 0;
}