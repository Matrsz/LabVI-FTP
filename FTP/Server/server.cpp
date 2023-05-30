#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <dirent.h>
#include "connections.h"
#include "filesystem.h"

void sendResponse(int socket, const std::string& response) {
    send(socket, response.c_str(), response.size(), 0);
}

std::vector<std::string> splitCommand(const std::string& command) {
    std::vector<std::string> parts;
    std::istringstream iss(command);
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }
    return parts;
}

std::string handleCWDCommand(int controlSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing directory argument.";
    } else {
        std::string directory = args;
        if (changeDirectory(directory)) {
            response = "Directory changed to " + getCurrentDirectory() + ".";
        } else {
            response = "Failed to change directory.";
        }
    }
    return response;
}

std::string handleCDUPCommand(int socket) {
    std::string response;
    if (changeToParentDirectory()) {
        response = "Directory changed to " + getCurrentDirectory() + ".";
    } else {
        response = "Failed to change to parent directory.";
    }
    return response;
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
            std::cout << "Received command: " << cmd << ", Args: " << args << std::endl;

            if (cmd == "LIST") {
                response = listEntries();
            } else if (cmd == "CWD") {
                response = handleCWDCommand(controlClientSocket, args);
            } else if (cmd == "CDUP") {
                response = handleCDUPCommand(controlClientSocket);
            } else {
                // Unsupported command
                response = "Unsupported command.";
            }
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
