#include <fstream>
#include <vector>
#include <sys/stat.h>
#include "connections.h"

bool fileExists(const std::string& filename);
void sendFile(int controlSocket, const std::string& filename);
void receiveFile(int controlSocket, const std::string& filename);
void receiveList(int controlSocket);
