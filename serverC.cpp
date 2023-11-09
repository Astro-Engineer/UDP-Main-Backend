#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>

#define SERVER_C_PORT 32994

std::unordered_map<std::string, std::vector<int>> dataMap;

std::string setupRead() {
    std::ifstream inputFile("dataC.txt"); 

    if (!inputFile) {
        std::cerr << "Failed to open the file." << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::string key;
        std::vector<int> values;

        key = line;

        std::getline(inputFile, line);
        std::istringstream valueStream(line);
        std::string valueStr;
        while (std::getline(valueStream, valueStr, ';')) {
        int value = std::stoi(valueStr);

            if (std::find(values.begin(), values.end(), value) == values.end()) {

                values.push_back(value);
            }
        }
        dataMap[key] = values;
    }

    inputFile.close();

    std::string concatenatedKeys;
    for (const auto& pair : dataMap) {
        if (!concatenatedKeys.empty()) {
            concatenatedKeys += ",";
        }
        concatenatedKeys += pair.first;
    }

    return concatenatedKeys;
};

void sendToMainServer(int serverASock, const char* message, int mainServerPort) {
    struct sockaddr_in mainServerAddr;
    memset(&mainServerAddr, 0, sizeof(mainServerAddr));

    mainServerAddr.sin_family = AF_INET;
    mainServerAddr.sin_port = htons(mainServerPort); 
    mainServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send the message to the Main Server
    if (sendto(serverASock, message, strlen(message), 0, (struct sockaddr*)&mainServerAddr, sizeof(mainServerAddr)) == -1) {
        perror("Error sending message to Main Server");
    }
}

int main() {
    int serverASock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverASock == -1) {
        perror("Error creating server C socket");
        exit(1);
    }

    struct sockaddr_in serverAAddr;
    memset(&serverAAddr, 0, sizeof(serverAAddr));
    serverAAddr.sin_family = AF_INET;
    serverAAddr.sin_port = htons(SERVER_C_PORT); 
    serverAAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverASock, (struct sockaddr*)&serverAAddr, sizeof(serverAAddr)) == -1) {
        perror("Error binding server C socket");
        exit(1);
    }
    std::cout<<"Server C is up and running using UDP on port "<< SERVER_C_PORT<<std::endl;     
    char buffer[1024];

        memset(buffer, 0, sizeof(buffer));
        recvfrom(serverASock, buffer, sizeof(buffer), 0, NULL, NULL);
        buffer[sizeof(buffer)] = '\0';

        std::string initMsg = setupRead();

        sendToMainServer(serverASock, initMsg.c_str(), 33994);
        std::cout<<"Server C has sent a department list to Main Server"<<std::endl;

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        recvfrom(serverASock, buffer, sizeof(buffer), 0, NULL, NULL);
        buffer[sizeof(buffer)] = '\0';
        std::cout<<"Server C has received a request for "<<buffer<<std::endl;
        std::stringstream ss;
        ss << "There are " << dataMap[buffer].size() << " distinct students in " << buffer << " department. Their IDs are ";
        std::string output = ss.str();
        std::cout<<"Server C found "<<dataMap[buffer].size()<<" distinct students for "<< buffer<< ":" <<std::to_string(dataMap[buffer][0]);
        output += std::to_string(dataMap[buffer][0]);
        for(int i=1; i<dataMap[buffer].size(); i++){
            std::cout<<"," + std::to_string(dataMap[buffer][i]);
            output += "," + std::to_string(dataMap[buffer][i]);
        }
        output += ".";
        sendToMainServer(serverASock, output.c_str(), 33994);
        printf("\nServer C has sent the results to Main Server");
        printf("\n%s\n", buffer);


        
    }
    close(serverASock);

    return 0;
}
