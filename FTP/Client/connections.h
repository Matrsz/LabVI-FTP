#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

void sendCommand(int socket, const std::string& command);
bool receiveResponse(int socket, std::string& response);
int createControlSocket(const std::string& serverIP, int serverPort);
int createDataSocket(const std::string& dataIP, int dataPort);
int establishDataConnection(int controlSocket);