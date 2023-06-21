#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>

void closeSocket(int socket) {
    // Set the socket to non-blocking mode
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    
    // Shutdown the socket to disable further send/receive operations
    shutdown(socket, SHUT_WR);
    
    // Read and discard any incoming data from the socket
    char buffer[1024];
    while (true) {
        ssize_t bytesRead = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
            break;
    }
    
    // Close the socket
    close(socket);
}

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
        closeSocket(controlSocket);
        return -1;
    }

    // Connect to the server
    if (connect(controlSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to the server." << std::endl;
        closeSocket(controlSocket);
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
        closeSocket(dataSocket);
        return -1;
    }

    return dataSocket;
}

int establishDataConnection(int controlSocket) {
    // Receive the server's IP address and port for the data connection
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(controlSocket, buffer, sizeof(buffer), 0);

    // Parse the server's IP address and port
    std::string response(buffer);
    size_t pos = response.find("PORT");
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
        closeSocket(controlSocket);
        return 1;
    }    
    return dataSocket;
}