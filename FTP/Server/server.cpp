#include <iostream>
#include <unistd.h>
#include <poll.h>

#include "connections.h"
#include "files.h"
#include "commands.h"

void handleClientConnection(int controlClientSocket) {
    sendWelcomeMessage(controlClientSocket);

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
            std::cout << "Got 0 bytes" << std::endl;
            break;
        }

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
                    std::cout << "Username: " << username << std::endl;
                } else if (cmd == "PASS") {
                    if (username == "") {
                        sendResponse(controlClientSocket, "Enter valid username first");
                        std::cout << "Username required" << std::endl;
                    } else {
                        authenticated = handlePASSCommand(controlClientSocket, args, username);
                        std::cout << "Authentication status: " << authenticated << std::endl;
                    }
                } else if (cmd == "QUIT") {
                    break;
                } else {
                    sendResponse(controlClientSocket, "Not signed in.");
                    std::cout << "Not signed in" << std::endl;
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

    // Close the control client socket
    closeSocket(controlClientSocket);
    std::cout << "Control client socket closed" << std::endl;
}

int main() {
    // Create a socket for the control connection
    int controlSocket = createSocket(2021);
    if (controlSocket == -1) {
        return 1;
    }

    std::cout << "Server is listening on port 2021..." << std::endl;

    // Prepare the pollfd structure for the control socket
    std::vector<pollfd> pollFds(1);
    pollFds[0].fd = controlSocket;
    pollFds[0].events = POLLIN;

    while (true) {
        int pollResult = poll(pollFds.data(), pollFds.size(), -1);
        if (pollResult == -1) {
            std::cerr << "Error occurred during poll()." << std::endl;
            break;
        }
        // Check if there is a new connection request on the control socket
        if (pollFds[0].revents & POLLIN) {
            pid_t pid = fork();
            if (pid < 0) {
                std::cerr << "Failed to fork a new process." << std::endl;
                return 1;
            } else if (pid == 0) {
                // Child process
                // Accept a client connection
                int controlClientSocket = acceptClientConnection(controlSocket);
                if (controlClientSocket == -1) {
                    return 1;
                }
                std::cout << "Client connected." << std::endl;
                closeSocket(controlSocket); // Close the control socket in the child process
                handleClientConnection(controlClientSocket);
                return 0;
            } else {
                // Parent process
                std::cout << "Parent process continuing to listen for new connections" << std::endl;
            }
        } else {
            usleep(10000);
        }
    }

    // Close the control socket (should not reach this point)
    closeSocket(controlSocket);

    return 0;
}
