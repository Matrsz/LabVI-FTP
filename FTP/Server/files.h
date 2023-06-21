#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <fstream>

std::string listEntries();
bool changeDirectory(const std::string& directory);
std::string getCurrentDirectory();
bool changeToParentDirectory();
bool makeDirectory(const std::string& directory);
bool removeDirectory(const std::string& directory);
bool deleteFile(const std::string& filename);
bool fileExists(const std::string& filename);
bool sendFile(int socket, const std::string& filename);
bool recvFile(int dataSocket, const std::string& filename);