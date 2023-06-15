#include "connections.h"
#include "filesystem.h"
#include "commands.h"

std::string handleUSERCommand(int controlClientSocket, const std::string& args) {
    std::string response;
    if (args.empty()) {
        response = "Missing username argument.";
    } else {
        std::string username = args;
        if (username == "admin") {
            response = "331 User name okay, need password.";
            sendResponse(controlClientSocket, response);
            return username;
        } 
        response = "530 Not logged in.";
    }
    sendResponse(controlClientSocket, response);
    return ""; // Return empty string if username is not validd
}

bool handlePASSCommand(int controlClientSocket, const std::string& args, const std::string& username) {
    std::string response;
    if (args.empty()) {
        response = "Missing password argument.";
    } else {
        std::string password = args;
        if (password == "password") {
            response = "230 User logged in, proceed.";
            sendResponse(controlClientSocket, response);
            return true; // Password is correct
        } 
        response = "530 Login incorrect.";
    }
    sendResponse(controlClientSocket, response);
    return false; // Password is incorrect
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

    sendWelcomeMessage(controlClientSocket);

    // Data connection is now ready for data transfer or other operations

    bool authenticated = false;
    std::string username = "";

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

            if (!authenticated) {
                if (cmd == "USER") {
                    username = handleUSERCommand(controlClientSocket, args);
                } else if (cmd == "PASS") {
                    if (username == "") {
                        sendResponse(controlClientSocket, "Enter valid username first");
                    } else {
                        authenticated = handlePASSCommand(controlClientSocket, args, username);
                    }
                } else {
                    sendResponse(controlClientSocket, "Not signed in.");
                }
            } else {
                if (cmd == "LIST") {
                    handleLISTCommand(controlClientSocket, args);
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
                    handleRETRCommand(controlClientSocket, args);
                } else if (cmd == "STOR") {
                    handleSTORCommand(controlClientSocket, args);
                } else if (cmd == "QUIT") {
                    break;
                } else {
                    sendResponse(controlClientSocket, "Unsupported command.");
                }
            }
        }
    }

    // Close the control client socket and control socket
    close(controlClientSocket);
    close(controlSocket);

    return 0;
}
