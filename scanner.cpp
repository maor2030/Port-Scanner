#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;
std::mutex mtx; // Declare a mutex

bool isPortOpen(const std::string& ipAddress, int port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    struct addrinfo* resultAddr = nullptr;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = getaddrinfo(ipAddress.c_str(), std::to_string(port).c_str(), &hints, &resultAddr);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << result << std::endl;
        WSACleanup();
        return false;
    }

    SOCKET connectSocket = socket(resultAddr->ai_family, resultAddr->ai_socktype, resultAddr->ai_protocol);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(resultAddr);
        WSACleanup();
        return false;
    }

    result = connect(connectSocket, resultAddr->ai_addr, static_cast<int>(resultAddr->ai_addrlen));
    if (result == SOCKET_ERROR) {
        closesocket(connectSocket);
        freeaddrinfo(resultAddr);
        WSACleanup();
        return false;
    }

    closesocket(connectSocket);
    freeaddrinfo(resultAddr);
    WSACleanup();
    return true;
}

void portScanner(const std::string& ipAddress, int startPort, int endPort) {
    for (int port = startPort; port <= endPort; ++port) {
        if (isPortOpen(ipAddress, port)) {
            std::lock_guard<std::mutex> guard(mtx); // Lock the mutex in a scoped manner
            std::cout << "Port " << port << " is open on " << ipAddress << std::endl;
        }
    }
}

int main() {
    std::string ipAddress;
    int startPort, endPort;
    cout << "Choose an ip to be scanned:";
    std::cin >> ipAddress;
    cout << "Choose a starting port:";
    std::cin >> startPort;
    cout << "Choose an end port:";
    std::cin >> endPort;
    cout << "scanning..." << endl;

    const int num_ports = endPort - startPort + 1;
    // Number of threads to create
    const int num_threads = min(num_ports/10+1,1000);

    // Vector to hold thread objects
    std::vector<std::thread> threads;

    // Calculate number of ports per thread
    int ports_per_thread = num_ports / num_threads;

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        int thread_start_port = startPort + i * ports_per_thread;
        int thread_end_port = thread_start_port + ports_per_thread - 1;
        threads.push_back(std::thread(portScanner, ipAddress, thread_start_port, thread_end_port));
    }

    // Join threads
    for (std::thread& t : threads) {
        t.join();
    }

    return 0;
}
