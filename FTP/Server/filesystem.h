#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <dirent.h>

std::string listEntries();
bool changeDirectory(const std::string& directory);
std::string getCurrentDirectory();
bool changeToParentDirectory();
bool makeDirectory(const std::string& directory);
bool removeDirectory(const std::string& directory);
bool deleteFile(const std::string& filename);
bool fileExists(const std::string& filename);