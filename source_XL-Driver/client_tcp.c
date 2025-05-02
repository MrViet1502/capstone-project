#include <winsock2.h>
#include <stdio.h>
#include <string.h>

#define SERVER_ADDR "127.0.0.1" // IP address of the server
#define SERVER_PORT 1234         // TCP port 1234

SOCKET connectToTcpServer() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error code: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed. Error code: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Set up the server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    serverAddr.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connect failed. Error code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    printf("Connected to server %s:%d\n", SERVER_ADDR, SERVER_PORT);
    return sock;
}
