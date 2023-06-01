#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <dirent.h>

void sendResponse(int socket, const std::string& response);
int createSocket(int port);
int acceptClientConnection(int socket);
void sendWelcomeMessage(int socket, const std::string& dataAddress, int dataPort);