#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

void sendResponse(int socket, const std::string& response);
int createSocket(int port);
int acceptClientConnection(int socket);
void sendWelcomeMessage(int socket);
int establishDataConnection(int controlClientSocket, int dataSocket, std::string &dataAddress, int dataPort);