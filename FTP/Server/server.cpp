#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <dirent.h>

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

void sendWelcomeMessage(int socket, const std::string& dataAddress, int dataPort) {
    std::string welcomeMessage = "220 Welcome to the FTP server.\r\n";
    send(socket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    std::string dataMessage = "PORT " + dataAddress + "," + std::to_string(dataPort) + "\r\n";
    send(socket, dataMessage.c_str(), dataMessage.size(), 0);
}

void sendResponse(int socket, const std::string& response) {
    send(socket, response.c_str(), response.size(), 0);
}

std::string listFiles() {
    std::string filelist;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                filelist += ent->d_name;
                filelist += "\r\n";
            }
        }
        closedir(dir);
    }
    return filelist;
}

int main() {
    // Create a socket for the control connection
    int controlSocket = createSocket(2021);
    if (controlSocket == -1) {
        return 1;
    }

    std::cout << "Server is listening on port 2021..." << std::endl;

    // Accept a client connection
    int controlClientSocket = acceptClientConnection(controlSocket);
    if (controlClientSocket == -1) {
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // Send welcome message to the client
    std::string dataAddress = "127.0.0.1";  // Replace with the actual server IP address
    int dataPort = 2022;  // Replace with the actual data port
    sendWelcomeMessage(controlClientSocket, dataAddress, dataPort);

    // Create a socket for the data connection
    int dataSocket = createSocket(dataPort);
    if (dataSocket == -1) {
        return 1;
    }

    std::cout << "Server is listening on port " << dataPort << " for data connections..." << std::endl;

    // Accept a data connection
    int dataClientSocket = acceptClientConnection(dataSocket);
    if (dataClientSocket == -1) {
        return 1;
    }

    std::cout << "Data connection established." << std::endl;

    // Data connection is now ready for data transfer or other operations

    // Enter command loop
    char commandBuffer[1024];
    while (true) {
        // Receive command from the client
        memset(commandBuffer, 0, sizeof(commandBuffer));
        ssize_t bytesRead = recv(controlClientSocket, commandBuffer, sizeof(commandBuffer), 0);
        if (bytesRead <= 0) {
            // Error or connection closed
            std::cout << "Connection closed." << std::endl;
            break;
        }

        // Process the command
        std::string command(commandBuffer);
        std::string response;

        // LIST command
        if (command.substr(0, 4) == "LIST") {
            response = listFiles();
        } else {
            // Unknown command
            response = "Unknown command: " + command + "\r\n";
        }

        // Send response back to the client
        sendResponse(controlClientSocket, response);
    }

    // Close the control client socket and control socket
    close(controlClientSocket);
    close(controlSocket);

    // Close the data client socket and data socket
    close(dataClientSocket);
    close(dataSocket);

    return 0;
}
