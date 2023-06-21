#include <fstream>
#include <vector>
#include <sys/stat.h>
#include "connections.h"

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}


void sendFile(int controlSocket, const std::string& filename) {
    // Wait for the server's response
    std::string response;
    if (!receiveResponse(controlSocket, response)) {
        std::cerr << "Failed to receive response from the server." << std::endl;
        return;
    }

    // Check if the server is ready to send the file
    if (response.substr(0, 3) != "150") {
        std::cerr << "Server rejected file retrieval: " << response << std::endl;
        return;
    }

    int dataSocket = establishDataConnection(controlSocket);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        closeSocket(dataSocket);
        return;
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

    file.close();

    ssize_t bytesSent = send(dataSocket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file contents." << std::endl;
        closeSocket(dataSocket);
        return;
    }
    
    closeSocket(dataSocket);
    receiveResponse(controlSocket, response);
}


void receiveFile(int controlSocket, const std::string& filename) {
    // Wait for the server's response
    std::string response;
    if (!receiveResponse(controlSocket, response)) {
        std::cerr << "Failed to receive response from the server." << std::endl;
        return;
    }

    // Check if the server is ready to send the file
    if (response.substr(0, 3) != "150") {
        std::cerr << "Server rejected file retrieval: " << response << std::endl;
        return;
    }

    int dataSocket = establishDataConnection(controlSocket);

    const int bufferSize = 1024; // Adjust the buffer size as needed
    std::vector<char> buffer(bufferSize);
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        closeSocket(dataSocket);
        return;
    }

    ssize_t bytesRead;
    while ((bytesRead = recv(dataSocket, buffer.data(), bufferSize, 0)) > 0) {
        file.write(buffer.data(), bytesRead);
    }

    if (bytesRead < 0) {
        std::cerr << "Failed to receive file contents." << std::endl;
        file.close();
        std::cout << "Closing Data Socket" << std::endl;
        closeSocket(dataSocket);
        return;
    }

    file.close();
    receiveResponse(controlSocket, response);
    closeSocket(dataSocket);
}


void receiveList(int controlSocket) {
    // Wait for the server's response
    std::string response;
    if (!receiveResponse(controlSocket, response)) {
        std::cerr << "Failed to receive response from the server." << std::endl;
        return;
    }

    // Check if the server is ready to send the file
    if (response.substr(0, 3) != "150") {
        std::cerr << "Server rejected file retrieval: " << response << std::endl;
        return;
    }
    int dataSocket = establishDataConnection(controlSocket);

    // Receive the file list from the data connection
    char buffer[1024];
    ssize_t bytesRead;
    std::string fileList;

    while ((bytesRead = recv(dataSocket, buffer, sizeof(buffer), 0)) > 0) {
        fileList += std::string(buffer, bytesRead);
    }

    // Print the received file list
    std::cout << "File List:\n" << fileList << std::endl;

    receiveResponse(controlSocket, response);
    
    closeSocket(dataSocket);
    return;
}
