#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

void sendResponse(int socket, const std::string& response) {
    std::cout << "Sending Response: " << response << std::endl;
    send(socket, response.c_str(), response.size(), 0);
}

int createSocket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 1) == -1) {
        std::cerr << "Failed to listen on socket." << std::endl;
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int acceptClientConnection(int socket) {
    sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(socket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        std::cerr << "Failed to accept client connection." << std::endl;
        close(socket);
        return -1;
    }

    return clientSocket;
}

void sendWelcomeMessage(int controlClientSocket) {
    std::string welcomeMessage = "220 Welcome to the FTP server.\r\n";
    send(controlClientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

}

int establishDataConnection(int controlClientSocket, int dataSocket, std::string &dataAddress, int dataPort){
    std::string dataMessage = "PORT " + dataAddress + "," + std::to_string(dataPort) + "\r\n";
    send(controlClientSocket, dataMessage.c_str(), dataMessage.size(), 0);

    if (dataSocket == -1) {
        std::cout << "Invalid Server Socket" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port " << dataPort << " for data connections..." << std::endl;

    // Accept a data connection
    int dataClientSocket = acceptClientConnection(dataSocket);
    if (dataClientSocket == -1) {
        std::cout << "Invalid Client Socket" << std::endl;
        return 1;
    }

    std::cout << "Data connection established." << std::endl;
    return dataClientSocket;
}