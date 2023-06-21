#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <dirent.h>
#include "files.h"
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
        response = "501 Missing directory argument.";
    } else {
        std::string directory = args;
        if (changeDirectory(directory)) {
            response = "212 Directory changed to " + getCurrentDirectory() + ".";
        } else {
            response = "550 Failed to change directory.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleCDUPCommand(int controlClientSocket) {
    std::string response;
    if (changeToParentDirectory()) {
        response = "212 Directory changed to " + getCurrentDirectory() + ".";
    } else {
        response = "550 Failed to change to parent directory.";
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleMKDCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "501 Missing directory name.";
    } else {
        std::string directory = args;
        if (makeDirectory(directory)) {
            response = "250 Directory '" + directory + "' created.";
        } else {
            response = "450 Failed to create directory '" + directory + "'.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}

void handleRMDCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "501 Missing directory name.";
    } else {
        std::string directory = args;
        if (removeDirectory(directory)) {
            response = "250 Directory '" + directory + "' removed.";
        } else {
            response = "550 Failed to remove directory '" + directory + "'.";
        }
    }
    sendResponse(controlClientSocket, response);
    return;
}


void handleDELECommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "501 Missing filename argument.";
    } else {
        std::string filename = args;
        if (deleteFile(filename)) {
            response = "250 File '" + filename + "' deleted.";
        } else {
            response = "550 Failed to delete file '" + filename + "'.";
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
        closeSocket(dataClientSocket);
        closeSocket(dataSocket);
    }
    sendResponse(controlClientSocket, response);
}

void handleRETRCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "501 Missing filename argument.";
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
            closeSocket(dataClientSocket);
            closeSocket(dataSocket);
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
        response = "501 Missing filename argument.";
    } else {
        std::string filename = args;
        sendResponse(controlClientSocket, "150 Opening data connection.\r\n");

        std::string dataAddress = "127.0.0.1";  // Replace with the actual server IP address   
        int dataPort = 2022;  // Replace with the actual data port
        int dataSocket = createSocket(dataPort);
        int dataClientSocket = establishDataConnection(controlClientSocket, dataSocket, dataAddress, dataPort);

        if (recvFile(dataClientSocket, filename)) {
            response = "226 File transfer successful.";
        } else {
            response = "451 File transfer failed.";
        }
        std::cout << "Closing Data Sockets" << std::endl;
        closeSocket(dataClientSocket);
        closeSocket(dataSocket);
    }
    sendResponse(controlClientSocket, response);
    return;
}

std::string handleUSERCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "501 Missing username argument.";
    } else {
        std::string username = args;
        if (username == "admin") {
            response = "331 User name okay, need password.";
            sendResponse(controlClientSocket, response);
            return username;
        } 
        response = "403 Invalid username.";
    }
    sendResponse(controlClientSocket, response);
    return ""; // Return empty string if username is not validd
}

bool handlePASSCommand(int controlClientSocket, const std::string& args, const std::string& username) {
    std::string response;
    if (args.empty()) {
        response = "501 Missing password argument.";
    } else {
        std::string password = args;
        if (password == "password") {
            response = "230 User logged in, proceed.";
            sendResponse(controlClientSocket, response);
            return true; // Password is correct
        } 
        response = "403 Login incorrect.";
    }
    sendResponse(controlClientSocket, response);
    return false; // Password is incorrect
}
