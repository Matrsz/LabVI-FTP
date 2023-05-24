#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    // Create a socket for the control connection
    int controlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (controlSocket == -1) {
        std::cerr << "Failed to create control socket." << std::endl;
        return 1;
    }

    // Set up the server address structure
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2021);  // FTP control port
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(controlSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind control socket." << std::endl;
        close(controlSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(controlSocket, 1) == -1) {
        std::cerr << "Failed to listen on control socket." << std::endl;
        close(controlSocket);
        return 1;
    }

    std::cout << "Server is listening on port 2021..." << std::endl;

    // Accept a client connection
    sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);
    int controlClientSocket = accept(controlSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (controlClientSocket == -1) {
        std::cerr << "Failed to accept client connection." << std::endl;
        close(controlSocket);
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // Send welcome message to the client
    std::string welcomeMessage = "220 Welcome to the FTP server.\r\n";
    send(controlClientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    // Send server IP address and port for data connection to the client
    std::string dataAddress = "127.0.0.1,2022";  // Replace with the actual server IP address and data port
    std::string dataMessage = "PORT " + dataAddress + "\r\n";
    send(controlClientSocket, dataMessage.c_str(), dataMessage.size(), 0);


    // Create a socket for the data connection
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket == -1) {
        std::cerr << "Failed to create data socket." << std::endl;
        return 1;
    }

    // Set up the server address structure for the data connection
    sockaddr_in dataServerAddress{};
    dataServerAddress.sin_family = AF_INET;
    dataServerAddress.sin_port = htons(2022);  // FTP data port
    dataServerAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the data socket to the server address
    if (bind(dataSocket, (struct sockaddr*)&dataServerAddress, sizeof(dataServerAddress)) == -1) {
        std::cerr << "Failed to bind data socket." << std::endl;
        close(dataSocket);
        return 1;
    }

    // Listen for incoming data connections
    if (listen(dataSocket, 1) == -1) {
        std::cerr << "Failed to listen on data socket." << std::endl;
        close(dataSocket);
        return 1;
    }

    std::cout << "Server is listening on port 2022 for data connections..." << std::endl;

    // Accept a data connection
    int dataClientSocket = accept(dataSocket, nullptr, nullptr);
    if (dataClientSocket == -1) {
        std::cerr << "Failed to accept data connection." << std::endl;
        close(dataSocket);
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
            std::cout << "Got 0 bytes";
            break;
        }

        // Process the command
        std::string command(commandBuffer);
        // TODO: Implement command processing logic

        // Send response back to the client
        std::string response = "Command received: " + command + "\r\n";
        send(controlClientSocket, response.c_str(), response.size(), 0);
    }
    
    // Close the control client socket and control socket
    close(controlClientSocket);
    close(controlSocket);

    // Close the data client socket and data socket
    close(dataClientSocket);
    close(dataSocket);

    return 0;
}
