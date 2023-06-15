#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <dirent.h>
#include "filesystem.h"
#include "connections.h"

std::vector<std::string> splitCommand(const std::string& command) {
    std::vector<std::string> parts;
    std::istringstream iss(command);
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }
    return parts;
}

void handleCWDCommand(int controlClientSocket, const std::string& args) {
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
    sendResponse(controlClientSocket, response);
    return;
}

void handleCDUPCommand(int controlClientSocket) {
    std::string response;
    if (changeToParentDirectory()) {
        response = "Directory changed to " + getCurrentDirectory() + ".";
    } else {
        response = "Failed to change to parent directory.";
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleMKDCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing directory name.";
    } else {
        std::string directory = args;
        if (makeDirectory(directory)) {
            response = "Directory '" + directory + "' created.";
        } else {
            response = "Failed to create directory '" + directory + "'.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleRMDCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing directory name.";
    } else {
        std::string directory = args;
        if (removeDirectory(directory)) {
            response = "Directory '" + directory + "' removed.";
        } else {
            response = "Failed to remove directory '" + directory + "'.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}


void handleDELECommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing file name.";
    } else {
        std::string filename = args;
        if (deleteFile(filename)) {
            response = "File '" + filename + "' deleted.";
        } else {
            response = "Failed to delete file '" + filename + "'.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleLISTCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        sendResponse(controlClientSocket, "150 Opening data connection.\r\n");
   
        std::string dataAddress = "127.0.0.1";  // Replace with the actual server IP address   
        int dataPort = 2022;  // Replace with the actual data port
        int dataSocket = createSocket(dataPort);
        int dataClientSocket = establishDataConnection(controlClientSocket, dataSocket, dataAddress, dataPort);
   
        std::string fileList = listEntries();
        if (!fileList.empty()) {
            ssize_t bytesSent = send(dataClientSocket, fileList.c_str(), fileList.size(), 0);
            if (bytesSent == -1) {
                response = "426 Connection closed; transfer aborted.";
            } else {
                response = "226 Closing data connection, sent " + std::to_string(bytesSent) + " bytes.";
            }
        } else {
            response = "550 Failed to list directory.";
        }
        close(dataClientSocket);
        close(dataSocket);
    }
    sendResponse(controlClientSocket, response);
}

void handleRETRCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing filename argument.";
    } else {
        std::string filename = args;
        if (fileExists(filename)) {
            sendResponse(controlClientSocket, "150 Opening data connection.\r\n");

            std::string dataAddress = "127.0.0.1";  // Replace with the actual server IP address   
            int dataPort = 2022;  // Replace with the actual data port
            int dataSocket = createSocket(dataPort);
            int dataClientSocket = establishDataConnection(controlClientSocket, dataSocket, dataAddress, dataPort);

            if (sendFile(dataClientSocket, filename)) {
                response = "226 File transfer successful.";
            } else {
                response = "451 File transfer failed.";
            }

        std::cout << "Closing Data Sockets" << std::endl;
            close(dataClientSocket);
            close(dataSocket);
        } else {
            response = "550 File not found.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleSTORCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing filename argument.";
    } else {
        std::string filename = args;
        sendResponse(controlClientSocket, "150 Opening data connection.\r\n");

        std::string dataAddress = "127.0.0.1";  // Replace with the actual server IP address   
        int dataPort = 2022;  // Replace with the actual data port
        int dataSocket = createSocket(dataPort);
        int dataClientSocket = establishDataConnection(controlClientSocket, dataSocket, dataAddress, dataPort);

        if (recvFile(dataSocket, filename)) {
            response = "226 File transfer successful.";
        } else {
            response = "451 File transfer failed.";
        }
        std::cout << "Closing Data Sockets" << std::endl;
        close(dataClientSocket);
        close(dataSocket);
    }
    sendResponse(controlClientSocket, response);
    return;
}