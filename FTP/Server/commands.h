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

void sendResponse(int socket, const std::string& response);
std::vector<std::string> splitCommand(const std::string& command);
void handleCWDCommand(int controlClientSocket, const std::string& args);
void handleCDUPCommand(int controlClientSocket);
void handleMKDCommand(int controlClientSocket, const std::string& args);
void handleRMDCommand(int controlClientSocket, const std::string& args);
void handleDELECommand(int controlClientSocket, const std::string& args);