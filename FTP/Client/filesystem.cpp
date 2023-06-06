#include <fstream>
#include <vector>
#include <sys/stat.h>
#include "connections.h"

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void sendFile(int controlSocket, const std::string& filename) {
    // Open the file for reading in binary mode
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file contents into a buffer
    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize)) {
        std::cerr << "Failed to read file: " << filename << std::endl;
        file.close();
        return;
    }

    file.close();

    std::cout << "File opened: " << filename << std::endl;
    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    // Wait for the server's response
    std::string response;
    receiveResponse(controlSocket, response);

    // Check if the server is ready to receive the file
    if (response.substr(0, 3) != "150") {
        std::cerr << "Server rejected file transfer: " << response << std::endl;
        return;
    }
    int dataSocket = establishDataConnection(controlSocket);

    // Send the file size to the server
    std::string fileSizeStr = std::to_string(fileSize);
    ssize_t bytesSent = send(dataSocket, fileSizeStr.c_str(), fileSizeStr.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file size." << std::endl;
        std::cout << "Closing Data Socket" << std::endl;
        close(dataSocket);
        return;
    }
    std::cout << "File size sent: " << bytesSent << " bytes" << std::endl;

    // Send the file contents to the server
    bytesSent = send(dataSocket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file contents." << std::endl;
        std::cout << "Closing Data Socket" << std::endl;
        close(dataSocket);
        return;
    }

    receiveResponse(controlSocket, response);

    std::cout << "Closing Data Socket" << std::endl;
    close(dataSocket);
    return;
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

    // Receive the file size from the server
    char fileSizeBuffer[1024];
    memset(fileSizeBuffer, 0, sizeof(fileSizeBuffer));
    ssize_t bytesRead = recv(dataSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file size." << std::endl;
        std::cout << "Closing Data Socket" << std::endl;
        close(dataSocket);
        return;
    }
    std::cout << "File size received: " << fileSizeBuffer << " bytes" << std::endl;

    std::string fileSizeStr(fileSizeBuffer, bytesRead);
    std::streamsize fileSize = std::stoi(fileSizeStr);

    // Receive the file contents from the server
    std::vector<char> buffer(fileSize);
    bytesRead = recv(dataSocket, buffer.data(), buffer.size(), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file contents." << std::endl;
        std::cout << "Closing Data Socket" << std::endl;
        close(dataSocket);
        return;
    }

    std::cout << "File contents received: " << bytesRead << " bytes" << std::endl;

    // Write the file contents to disk
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return;
    }
    file.write(buffer.data(), bytesRead);
    file.close();

    receiveResponse(controlSocket, response);
    
    std::cout << "Closing Data Socket" << std::endl;
    close(dataSocket);
    return;
}
