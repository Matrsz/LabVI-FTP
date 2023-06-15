#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <dirent.h>
#include "filesystem.h"
#include "connections.h"

void sendResponse(int socket, const std::string& response);
std::vector<std::string> splitCommand(const std::string& command);
void handleCWDCommand(int controlClientSocket, const std::string& args);
void handleCDUPCommand(int controlClientSocket);
void handleMKDCommand(int controlClientSocket, const std::string& args);
void handleRMDCommand(int controlClientSocket, const std::string& args);
void handleDELECommand(int controlClientSocket, const std::string& args);
void handleRETRCommand(int controlClientSocket, const std::string& args);
void handleSTORCommand(int controlClientSocket, const std::string& args);
void handleLISTCommand(int controlClientSocket, const std::string& args);