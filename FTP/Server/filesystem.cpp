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

    // Get the file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file contents into a buffer
    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize)) {
        std::cerr << "Failed to read file: " << filename << std::endl;
        file.close();
        return false;
    }

    file.close();

    std::cout << "File opened: " << filename << std::endl;
    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    // Send the file size to the client
    std::string fileSizeStr = std::to_string(fileSize);
    ssize_t bytesSent = send(socket, fileSizeStr.c_str(), fileSizeStr.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file size." << std::endl;
        return false;
    }
    std::cout << "File size sent: " << bytesSent << " bytes" << std::endl;

    // Send the file contents to the client
    bytesSent = send(socket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send file contents." << std::endl;
        return false;
    }
    std::cout << "File contents sent: " << bytesSent << " bytes" << std::endl;

    std::cout << "File sent successfully: " << filename << std::endl;
    return true;
}

bool recvFile(int dataSocket, const std::string& filename) {
        // Receive the file size from the server
    char fileSizeBuffer[1024];
    memset(fileSizeBuffer, 0, sizeof(fileSizeBuffer));
    ssize_t bytesRead = recv(dataSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file size." << std::endl;
        return false;
    }
    std::cout << "File size received: " << fileSizeBuffer << " bytes" << std::endl;

    std::string fileSizeStr(fileSizeBuffer, bytesRead);
    std::streamsize fileSize = std::stoi(fileSizeStr);

    // Receive the file contents from the server
    std::vector<char> buffer(fileSize);
    bytesRead = recv(dataSocket, buffer.data(), buffer.size(), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive file contents." << std::endl;
        return false;
    }

    std::cout << "File contents received: " << bytesRead << " bytes" << std::endl;

    // Write the file contents to disk
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return false;
    }

    file.write(buffer.data(), bytesRead);
    file.close();
    return true;
}