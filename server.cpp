/*
 * K Vivek Kumar
 * Reference: https://www.cs.uic.edu/~troy/fall99/eecs471/sockets.html
 */

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include <netdb.h>

#include "server_config.h"

int *currentSocketIds = new int[MAX_USERS];
int currentUserIndex = 0;

void handleClient(int clientSocketId)
{
    while (true)
    {
        char buffer[MSG_BUFFER_SIZE];
        int receivedStatus = recv(clientSocketId, buffer, MSG_BUFFER_SIZE, 0);
        if (receivedStatus < 0)
        {
            std::cout << "Failed to receive message" << std::endl;
            continue;
        }
        if (receivedStatus == 0)
        {
            std::cout << "Client (" << clientSocketId << ") closed the connection" << std::endl;
            close(clientSocketId);
            break;
        }
        for (int i = 0; i < currentUserIndex; i++)
        {
            if (currentSocketIds[i] != clientSocketId)
            {
                int sendStatus = send(currentSocketIds[i], buffer, receivedStatus, 0);
                if (sendStatus == -1)
                {
                    std::cout << "Failed to send message" << std::endl;
                }
            }
        }
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
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    int bindStatus = bind(socketId, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindStatus == -1)
    {
        std::cout << "Binding failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is running on port: " << EXPOSED_PORT << std::endl;

    int listeningStatus = listen(socketId, MAX_USER_SOCKETS);
    if (listeningStatus == -1)
    {
        std::cout << "Failed to listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocketId = accept(socketId, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if (clientSocketId == -1)
        {
            std::cout << "Client connection failed" << std::endl;
            continue;
        }

        std::cout << "Client connected from: " << inet_ntoa(clientAddress.sin_addr) << ":" << clientAddress.sin_port << std::endl;

        currentSocketIds[currentUserIndex++] = clientSocketId;

        std::thread clientThread(handleClient, clientSocketId);
        clientThread.detach();
    }

    close(socketId);
    return 0;
}
