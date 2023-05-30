#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

int main() {
    // Create a socket for the control connection
    int controlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (controlSocket == -1) {
        std::cerr << "Failed to create control socket." << std::endl;
        return 1;
    }

    // Set up the server address structure
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2021);  // FTP control port

    // Convert server IP address from string to binary format
    std::string serverIP = "127.0.0.1";  // Replace with the actual server IP address
    serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str());
    if (serverAddress.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Invalid server IP address." << std::endl;
        close(controlSocket);
        return 1;
    }

    // Connect to the server
    if (connect(controlSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to the server." << std::endl;
        close(controlSocket);
        return 1;
    }

    // Receive welcome message from the server
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(controlSocket, buffer, sizeof(buffer), 0);
    std::cout << "Received: " << buffer;

    // Receive the server's IP address and port for the data connection
    memset(buffer, 0, sizeof(buffer));
    recv(controlSocket, buffer, sizeof(buffer), 0);
    std::cout << "Received: " << buffer;

    // Parse the server's IP address and port
    std::string response(buffer);
    size_t pos = response.find("PORT");
    if (pos != std::string::npos) {
        std::string dataAddress = response.substr(pos + 5);
        std::string dataIP;
        int dataPort = 0;

        // Parse the IP address and port from the response
        // The format should be "IP1,IP2,IP3,IP4,Port1,Port2"
        // Extract IP address
        pos = dataAddress.find_last_of(",");
        dataIP = dataAddress.substr(0, pos);
        // Extract port
        dataPort = std::stoi(dataAddress.substr(pos + 1));

        // Set up the server address structure for the data connection
        sockaddr_in dataServerAddress{};
        dataServerAddress.sin_family = AF_INET;
        dataServerAddress.sin_port = htons(dataPort);
        dataServerAddress.sin_addr.s_addr = inet_addr(dataIP.c_str());

        // Create a socket for the data connection
        int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (dataSocket == -1) {
            std::cerr << "Failed to create data socket." << std::endl;
            close(controlSocket);
            return 1;
        }

        // Connect to the server's data socket
        if (connect(dataSocket, (struct sockaddr*)&dataServerAddress, sizeof(dataServerAddress)) == -1) {
            std::cerr << "Failed to connect to the server's data socket." << std::endl;
            close(dataSocket);
            close(controlSocket);
            return 1;
        }

        // Data connection established, you can now proceed with data transfer or other operations

        // Command loop
        std::string command;
        while (true) {
            // Read command from stdin
            std::cout << "Enter a command: ";
            std::getline(std::cin, command);

            // Send command to the server
            send(controlSocket, command.c_str(), command.size(), 0);

            // Receive response from the server
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recv(controlSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                // Error or connection closed
                break;
            }

            // Display the response
            std::cout << "Response: " << buffer << std::endl;

            // Check for termination command
            if (command == "QUIT") {
                break;
            }
        }
        // Close the data socket
        close(dataSocket);

    }

    // Close the control socket
    close(controlSocket);

    return 0;
}
