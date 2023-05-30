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