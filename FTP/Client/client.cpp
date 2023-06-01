#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>
#include <vector>

void sendCommand(int socket, const std::string& command) {
    send(socket, command.c_str(), command.size(), 0);
    return;
}

bool receiveResponse(int socket, std::string& response) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(socket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive response." << std::endl;
        return false;
    }
    response = std::string(buffer, bytesRead);
    std::cout << "Response: " << buffer << std::endl;
    return true;
}

void sendFile(int controlSocket, int dataSocket, const std::string& filename) {
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

    // Send the file size to the server
    std::string fileSizeStr = std::to_string(fileSize);
    ssize_t bytesSent = send(dataSocket, fileSizeStr.c_str(), fileSizeStr.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file size." << std::endl;
        return;
    }
    std::cout << "File size sent: " << bytesSent << " bytes" << std::endl;

    // Send the file contents to the server
    bytesSent = send(dataSocket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file contents." << std::endl;
        return;
    }

    return;
}

void receiveFile(int controlSocket, int dataSocket, const std::string& filename) {
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

    // Receive the file size from the server
    char fileSizeBuffer[1024];
    memset(fileSizeBuffer, 0, sizeof(fileSizeBuffer));
    ssize_t bytesRead = recv(dataSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file size." << std::endl;
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

    return;
}

int createControlSocket(const std::string& serverIP, int serverPort) {
    // Create a socket for the control connection
    int controlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (controlSocket == -1) {
        std::cerr << "Failed to create control socket." << std::endl;
        return -1;
    }

    // Set up the server address structure
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    // Convert server IP address from string to binary format
    serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str());
    if (serverAddress.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Invalid server IP address." << std::endl;
        close(controlSocket);
        return -1;
    }

    // Connect to the server
    if (connect(controlSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to the server." << std::endl;
        close(controlSocket);
        return -1;
    }

    return controlSocket;
}

int createDataSocket(const std::string& dataIP, int dataPort) {
    // Set up the server address structure for the data connection
    sockaddr_in dataServerAddress{};
    dataServerAddress.sin_family = AF_INET;
    dataServerAddress.sin_port = htons(dataPort);
    dataServerAddress.sin_addr.s_addr = inet_addr(dataIP.c_str());

    // Create a socket for the data connection
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket == -1) {
        std::cerr << "Failed to create data socket." << std::endl;
        return -1;
    }

    // Connect to the server's data socket
    if (connect(dataSocket, (struct sockaddr*)&dataServerAddress, sizeof(dataServerAddress)) == -1) {
        std::cerr << "Failed to connect to the server's data socket." << std::endl;
        close(dataSocket);
        return -1;
    }

    return dataSocket;
}

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

        // Create the data socket and connect to the server's data socket
        int dataSocket = createDataSocket(dataIP, dataPort);
        if (dataSocket == -1) {
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
            std::cout << "Sent Command: " << command << std::endl;

            // Check for termination command
            if (command == "QUIT") {
                break;
            } else if (command.substr(0, 4) == "RETR") {
                // Extract the filename from the command
                std::string filename = command.substr(5);
                std::cout << "Will receive file: " << filename << std::endl;
                receiveFile(controlSocket, dataSocket, filename);
            } else if (command.substr(0, 4) == "STOR") {
                // Extract the filename from the command
                std::string filename = command.substr(5);
                std::cout << "Will send file: " << filename << std::endl;
                sendFile(controlSocket, dataSocket, filename);
            }
            // Receive response from the server
            receiveResponse(controlSocket, response);
        }

        // Close the data socket
        close(dataSocket);
    }

    // Close the control socket
    close(controlSocket);

    return 0;
}
