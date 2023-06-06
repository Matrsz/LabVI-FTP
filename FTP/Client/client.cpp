#include "connections.h"
#include "filesystem.h"

int main() {
    // Create a socket for the control connection
    std::string serverIP = "127.0.0.1";  // Replace with the actual server IP address
    int controlSocket = createControlSocket(serverIP, 2021);
    if (controlSocket == -1) {
        return 1;
    }

    // Receive welcome message from the server
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(controlSocket, buffer, sizeof(buffer), 0);
    std::cout << "Received: " << buffer;

    // Command loop
    std::string command;
    std::string response(buffer);
    while (true) {
        // Read command from stdin
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        // Check for termination command
        if (command == "QUIT") {
            sendCommand(controlSocket, command);
            break;
        } else if (command.substr(0, 4) == "RETR") {
            // Extract the filename from the command
            sendCommand(controlSocket, command);
            std::string filename = command.substr(5);
            std::cout << "Will receive file: " << filename << std::endl;
            receiveFile(controlSocket, filename);
        } else if (command.substr(0, 4) == "STOR") {
            // Extract the filename from the command
            std::string filename = command.substr(5);
            if (fileExists(filename)) {
                std::cout << "Will send file: " << filename << std::endl;
                sendCommand(controlSocket, command);
                sendFile(controlSocket, filename);
            }
        } else {
            sendCommand(controlSocket, command);
            receiveResponse(controlSocket, response);
        }
    }
    close(controlSocket);
    return 0;
}
