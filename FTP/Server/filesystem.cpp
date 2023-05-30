#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <dirent.h>

std::string listEntries() {
    std::string entryList;
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string entryName = ent->d_name;
            entryList += entryName + "\r\n";
        }
        closedir(dir);
    } else {
        entryList = "Failed to read directory.";
    }
    return entryList;
}

bool changeDirectory(const std::string& directory) {
    if (chdir(directory.c_str()) == -1) {
        return false;
    }
    return true;
}

std::string getCurrentDirectory() {
    char currentDir[PATH_MAX];
    if (getcwd(currentDir, sizeof(currentDir)) != NULL) {
        return currentDir;
    }
    return "Failed to get current directory.";
}

bool changeToParentDirectory() {
    std::string currentDir = getCurrentDirectory();
    size_t lastSlashPos = currentDir.find_last_of('/');
    if (lastSlashPos == std::string::npos) {
        std::cerr << "Failed to determine parent directory." << std::endl;
        return false;
    }
    std::string parentDir = currentDir.substr(0, lastSlashPos);
    return changeDirectory(parentDir);
}

bool makeDirectory(const std::string& directory) {
    int result = mkdir(directory.c_str(), 0755);
    return (result == 0);
}

bool removeDirectory(const std::string& directory) {
    int result = rmdir(directory.c_str());
    return (result == 0);
}

bool deleteFile(const std::string& filename) {
    int result = unlink(filename.c_str());
    return (result == 0);
}