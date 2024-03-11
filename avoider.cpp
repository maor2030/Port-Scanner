#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // Include this header for inet_pton and INET_ADDRSTRLEN

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib
#define IP "192.168"
#define PORT 1234

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.substr(0, prefix.length()) == prefix;
}

int main() {
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT); // Port number you want to bind to
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to all available interfaces

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Bound to port "<<PORT<<" successfully.\n";

    char serverIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), serverIP, INET_ADDRSTRLEN);

    while (true) {
        if (startsWith(serverIP,IP)==0) {
            if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
                std::cerr << "Listen failed.\n";
                closesocket(serverSocket);
                WSACleanup();
                return 1;
            }
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed.\n";
                closesocket(serverSocket);
                WSACleanup();
                return 1;
            }
            else {
                std::cout << "Accepted connection.\n";
                // Now you can communicate with the client using clientSocket
                // Example: send(clientSocket, "Hello", 5, 0);
                closesocket(clientSocket);
            }
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}