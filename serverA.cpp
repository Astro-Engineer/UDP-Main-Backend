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

#define SERVER_A_PORT 30994
std::unordered_map<std::string, std::vector<int>> dataMap;
//open file and fill map with relevent data
std::string setupRead() {
    std::ifstream inputFile("dataA.txt"); 

    if (!inputFile) {
        std::cerr << "Failed to open the file." << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::string key;
        std::vector<int> values;
        //read key
        key = line;
        std::getline(inputFile, line);
        std::istringstream valueStream(line);
        std::string valueStr;
        while (std::getline(valueStream, valueStr, ';')) {
        int value = std::stoi(valueStr);
            //check for duplicates
            if (std::find(values.begin(), values.end(), value) == values.end()) {
                values.push_back(value);
            }
        }
        dataMap[key] = values;
    }
    
    inputFile.close();
    //prepare string to send relevant data to main server
    std::string concatenatedKeys;
    for (const auto& pair : dataMap) {
        if (!concatenatedKeys.empty()) {
            concatenatedKeys += ",";
        }
        concatenatedKeys += pair.first;
    }
    return concatenatedKeys;
};

//functionfor sending message to main server
void sendToMainServer(int serverASock, const char* message, int mainServerPort) {
    struct sockaddr_in mainServerAddr;
    memset(&mainServerAddr, 0, sizeof(mainServerAddr));

    mainServerAddr.sin_family = AF_INET;
    //setting up port
    mainServerAddr.sin_port = htons(mainServerPort); 
    //settting up ip
    mainServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    //sending message with error checking
    if (sendto(serverASock, message, strlen(message), 0, (struct sockaddr*)&mainServerAddr, sizeof(mainServerAddr)) == -1) {
        perror("Error sending message to Main Server");
    }
}

int main() {
    //create a socket
    int serverASock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverASock == -1) {
        perror("Error creating server A socket");
        exit(1);
    }

    //set up address
    struct sockaddr_in serverAAddr;
    memset(&serverAAddr, 0, sizeof(serverAAddr));
    serverAAddr.sin_family = AF_INET;
    serverAAddr.sin_port = htons(SERVER_A_PORT);
    //used the suggestion from https://www.beej.us/guide/bgnet/html/split/man-pages.html#man-pages
    serverAAddr.sin_addr.s_addr = INADDR_ANY;

    //bind socket to address
    //https://www.beej.us/guide/bgnet/html/split/man-pages.html#bindman
    if (bind(serverASock, (struct sockaddr*)&serverAAddr, sizeof(serverAAddr)) == -1) {
        perror("Error binding server A socket");
        exit(1);
    }
    std::cout<<"Server A is up and running using UDP on port "<< SERVER_A_PORT<<std::endl;     
    char buffer[1024];
        //recieve message from main to "wakeup" the backend
        memset(buffer, 0, sizeof(buffer));
        recvfrom(serverASock, buffer, sizeof(buffer), 0, NULL, NULL);
        buffer[sizeof(buffer)] = '\0';

        //create map from data values
        std::string initMsg = setupRead();
        //send map contents to main server
        sendToMainServer(serverASock, initMsg.c_str(), 33994);

        std::cout<<"Server A has sent a department list to Main Server"<<std::endl;
    while (1) {
        //constantly check and loop through, awaiting messages to respond to
        memset(buffer, 0, sizeof(buffer));
        recvfrom(serverASock, buffer, sizeof(buffer), 0, NULL, NULL);
        buffer[sizeof(buffer)] = '\0';
        std::cout<<"Server A has received a request for "<<buffer<<std::endl;
        std::stringstream ss;
        ss << "There are " << dataMap[buffer].size() << " distinct students in " << buffer << " department. Their IDs are ";
        std::string output = ss.str();
        std::cout<<"Server A found "<<dataMap[buffer].size()<<" distinct students for "<< buffer<< ":" <<std::to_string(dataMap[buffer][0]);
        output += std::to_string(dataMap[buffer][0]);
        for(int i=1; i<dataMap[buffer].size(); i++){
            std::cout<<"," + std::to_string(dataMap[buffer][i]);
            output += "," + std::to_string(dataMap[buffer][i]);
        }
        output += ".";
        sendToMainServer(serverASock, output.c_str(), 33994);
        printf("\nServer A has sent the results to Main Server");
        printf("\n%s\n", buffer);
    }
    //close socket
    close(serverASock);

    return 0;
}
