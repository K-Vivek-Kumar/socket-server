/*
 * K Vivek Kumar
 * Reference: https://www.cs.uic.edu/~troy/fall99/eecs471/sockets.html
 */

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include <cstring>

#include "client_config.h"

void handleServerResponse(int clientSocketId)
{
    char buffer[MSG_BUFFER_SIZE];
    while (true)
    {
        int receivedStatus = recv(clientSocketId, buffer, MSG_BUFFER_SIZE, 0);
        if (receivedStatus < 0)
        {
            std::cout << "Failed to receive message from server" << std::endl;
            continue;
        }
        if (receivedStatus == 0)
        {
            std::cout << "Server closed the connection" << std::endl;
            break;
        }

        buffer[receivedStatus] = '\0';
        std::cout << "\nServer (" << receivedStatus << "): " << buffer << std::endl;
    }
}

int main()
{
    int socketId = socket(AF_INET, SOCK_STREAM, 0);
    if (socketId == -1)
    {
        std::cout << "Socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(EXPOSED_PORT);
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);

    int connectionStatus = connect(socketId, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectionStatus == -1)
    {
        std::cout << "Failed to connect to server" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected to server at 127.0.0.1:8080" << std::endl;

    std::thread serverResponseThread(handleServerResponse, socketId);

    while (true)
    {
        char message[MSG_BUFFER_SIZE];
        std::cout << "Enter message: ";
        std::cin.getline(message, MSG_BUFFER_SIZE);

        if (strcmp(message, "exit") == 0)
        {
            std::cout << "Connection closing" << std::endl;
            break;
        }

        int sendStatus = send(socketId, message, strlen(message), 0);
        if (sendStatus == -1)
        {
            std::cout << "Failed to send message" << std::endl;
        }
    }

    close(socketId);
    serverResponseThread.join();
    std::cout << "Connection closed" << std::endl;

    return 0;
}
