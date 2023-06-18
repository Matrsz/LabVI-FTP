#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <fstream>

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

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

bool sendFile(int socket, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // Read the file contents into a buffer
    std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

    file.close();

    std::cout << "File opened: " << filename << std::endl;
    std::cout << "File size: " << buffer.size() << " bytes" << std::endl;

    // Send the file contents to the client
    ssize_t bytesSent = send(socket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file contents." << std::endl;
        return false;
    }
    std::cout << "File contents sent: " << bytesSent << " bytes" << std::endl;

    std::cout << "File sent successfully: " << filename << std::endl;
    return true;
}


bool recvFile(int dataSocket, const std::string& filename) {
    const int bufferSize = 1024; // Adjust the buffer size as needed

    std::vector<char> buffer(bufferSize);
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return false;
    }

    ssize_t bytesRead;
    while ((bytesRead = recv(dataSocket, buffer.data(), bufferSize, 0)) > 0) {
        file.write(buffer.data(), bytesRead);
    }

    if (bytesRead < 0) {
        std::cerr << "Failed to receive file contents." << std::endl;
        file.close();
        std::cout << "Closing Data Socket" << std::endl;
        return false;
    }

    file.close();

    std::cout << "File received successfully: " << filename << std::endl;

    return true;
}