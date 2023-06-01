#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <fstream>
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