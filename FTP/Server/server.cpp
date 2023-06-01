#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <fstream>
#include "connections.h"
#include "filesystem.h"
#include "commands.h"


bool sendFile(int socket, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
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
        return false;
    }

    file.close();

    std::cout << "File opened: " << filename << std::endl;
    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    // Send the file size to the client
    std::string fileSizeStr = std::to_string(fileSize);
    ssize_t bytesSent = send(socket, fileSizeStr.c_str(), fileSizeStr.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file size." << std::endl;
        return false;
    }
    std::cout << "File size sent: " << bytesSent << " bytes" << std::endl;

    // Send the file contents to the client
    bytesSent = send(socket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file contents." << std::endl;
        return false;
    }
    std::cout << "File contents sent: " << bytesSent << " bytes" << std::endl;

    std::cout << "File sent successfully: " << filename << std::endl;
    return true;
}

bool recvFile(int dataSocket, const std::string& filename) {
        // Receive the file size from the server
    char fileSizeBuffer[1024];
    memset(fileSizeBuffer, 0, sizeof(fileSizeBuffer));
    ssize_t bytesRead = recv(dataSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file size." << std::endl;
        return false;
    }
    std::cout << "File size received: " << fileSizeBuffer << " bytes" << std::endl;

    std::string fileSizeStr(fileSizeBuffer, bytesRead);
    std::streamsize fileSize = std::stoi(fileSizeStr);

    // Receive the file contents from the server
    std::vector<char> buffer(fileSize);
    bytesRead = recv(dataSocket, buffer.data(), buffer.size(), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file contents." << std::endl;
        return false;
    }

    std::cout << "File contents received: " << bytesRead << " bytes" << std::endl;

    // Write the file contents to disk
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return false;
    }

    file.write(buffer.data(), bytesRead);
    file.close();
    return true;
}

void handleRETRCommand(int controlSocket, int dataSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing filename argument.";
    } else {
        std::string filename = args;
        if (fileExists(filename)) {
            sendResponse(controlSocket, "150 Opening data connection.\r\n");
            if (sendFile(dataSocket, filename)) {
                response = "226 File transfer successful.";
            } else {
                response = "451 File transfer failed.";
            }
        } else {
            response = "550 File not found.";
        }
    }
    sendResponse(controlSocket, response);
    return;
}

void handleSTORCommand(int controlSocket, int dataSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing filename argument.";
    } else {
        std::string filename = args;
        sendResponse(controlSocket, "150 Opening data connection.\r\n");
        if (recvFile(dataSocket, filename)) {
            response = "226 File transfer successful.";
        } else {
            response = "451 File transfer failed.";
        }
    }
    sendResponse(controlSocket, response);
    return;
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
            std::cout << "Got 0 bytes";
            break;
        }

        // Process the command
        std::string command(commandBuffer);
        std::string response;

        // Split command into command proper and arguments
        std::vector<std::string> commandParts = splitCommand(command);
        if (commandParts.empty()) {
            response = "Invalid command.";
        } else {
            std::string cmd = commandParts[0];
            std::string args;
            if (commandParts.size() > 1) {
                args = command.substr(cmd.length() + 1);
            }

            // Print command details to stdout
            std::cout << "Received command: " << cmd << "\t" << args << std::endl;

            if (cmd == "LIST") {
                sendResponse(controlClientSocket, listEntries());
            } else if (cmd == "CWD") {
                handleCWDCommand(controlClientSocket, args);
            } else if (cmd == "CDUP") {
                handleCDUPCommand(controlClientSocket);            
            } else if (cmd == "MKD") {
                handleMKDCommand(controlClientSocket, args);
            } else if (cmd == "RMD") {
                handleRMDCommand(controlClientSocket, args);
            } else if (cmd == "DELE") {
                handleDELECommand(controlClientSocket, args);
            } else if (cmd == "RETR") {
                handleRETRCommand(controlClientSocket, dataClientSocket, args);
            } else if (cmd == "STOR") {
                handleSTORCommand(controlClientSocket, dataClientSocket, args);
            } else if (cmd == "QUIT") {
                break;
            } else {
                sendResponse(controlClientSocket, "Unsupported command.");
            }
        }
    }

    // Close the control client socket and control socket
    close(controlClientSocket);
    close(controlSocket);

    // Close the data client socket and data socket
    close(dataClientSocket);
    close(dataSocket);

    return 0;
}
