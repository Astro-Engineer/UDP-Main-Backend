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

#define MAIN_SERVER_PORT 33994

//get string and crete map to store all the data 
void parseStringToMap(const char* input, std::unordered_map<std::string, int>& stringMap, int server) {
    if (input) {
        std::string inputString(input);
        std::istringstream ss(inputString);
        std::string token;

        while (std::getline(ss, token, ',')) {
            stringMap[token] = server;
        }
    }
}

//function to send data to specific server
void sendToServer(int serverSock, const char* message, int ServerPort) {
    struct sockaddr_in mainServerAddr;
    memset(&mainServerAddr, 0, sizeof(mainServerAddr));
    //same sources as backend code
    mainServerAddr.sin_family = AF_INET;
    mainServerAddr.sin_port = htons(ServerPort); 
    mainServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if (sendto(serverSock, message, strlen(message), 0, (struct sockaddr*)&mainServerAddr, sizeof(mainServerAddr)) == -1) {
        perror("Error sending message to Main Server");
    }
}

int main() {
    //create a socket
    int mainServerSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (mainServerSock == -1) {
        perror("Error creating main server socket");
        exit(1);
    }

    //set up address
    struct sockaddr_in mainServerAddr;
    memset(&mainServerAddr, 0, sizeof(mainServerAddr));
    mainServerAddr.sin_family = AF_INET;
    mainServerAddr.sin_port = htons(MAIN_SERVER_PORT); 
    //used the suggestion from https://www.beej.us/guide/bgnet/html/split/man-pages.html#man-pages
    mainServerAddr.sin_addr.s_addr = INADDR_ANY;

    //bind socket to address
    //https://www.beej.us/guide/bgnet/html/split/man-pages.html#bindman
    if (bind(mainServerSock, (struct sockaddr*)&mainServerAddr, sizeof(mainServerAddr)) == -1) {
        perror("Error binding main server socket");
        exit(1);
    }

    std::cout<<"Main server is up and running."<<std::endl;

    char buffer[1024];
        //recieve messages from each server and add to total map
        std::unordered_map<std::string, int> map;
        sendToServer(mainServerSock, "", 30994);

        memset(buffer, 0, sizeof(buffer));
        recvfrom(mainServerSock, buffer, sizeof(buffer), 0, NULL, NULL);

        parseStringToMap(buffer, map, 0);
        printf("Main server has received the department list from Backend server A using UDP over port %i\n", MAIN_SERVER_PORT);

        sendToServer(mainServerSock, "", 31994);

        memset(buffer, 0, sizeof(buffer));
        recvfrom(mainServerSock, buffer, sizeof(buffer), 0, NULL, NULL);

        printf("Main server has received the department list from Backend server B using UDP over port %i\n", MAIN_SERVER_PORT);
        parseStringToMap(buffer, map, 1);

        sendToServer(mainServerSock, "", 32994);

        memset(buffer, 0, sizeof(buffer));
        recvfrom(mainServerSock, buffer, sizeof(buffer), 0, NULL, NULL);

        printf("Main server has received the department list from Backend server C using UDP over port %i\n", MAIN_SERVER_PORT);
        parseStringToMap(buffer, map, 2);
        //print all data in map
        std::cout<<"Server A"<<std::endl;
        for (const auto& pair : map) {
            if(pair.second == 0){
                std::cout<<pair.first<<std::endl;
            }
        }

        std::cout<<"Server B"<<std::endl;
        for (const auto& pair : map) {
            if(pair.second == 1){
                std::cout<<pair.first<<std::endl;
            }
        }

        std::cout<<"Server C"<<std::endl;
        for (const auto& pair : map) {
            if(pair.second == 2){
                std::cout<<pair.first<<std::endl;
            }
        }

    while (1) {
        //loop to query and recieve/send data
        std::cout <<std::endl<< "Enter Department Name: ";
        std::string in;
        std::cin >> in;
        auto it = map.find(in);

        if (it != map.end()) {
        if(it->second == 0){
            std::cout<<in<< " shows up in server A"<<std::endl;
            sendToServer(mainServerSock, it->first.c_str(), 30994);
            std::cout<<"The Main Server has sent request for " <<in<< " to server A using UDP over port "<<MAIN_SERVER_PORT<<std::endl;
            memset(buffer, 0, sizeof(buffer));
            //https://www.beej.us/guide/bgnet/html/split/system-calls-or-bust.html#sendtorecv
            recvfrom(mainServerSock, buffer, sizeof(buffer), 0, NULL, NULL);
            std::cout<<"The Main server has received searching result(s) of "<<in<<" from Backend server A"<<std::endl;
            //make sure buffer has end character
            buffer[sizeof(buffer)] = '\0';
            printf("%s\n", buffer);

        }
        else if(it->second == 1){
            std::cout<<in<< " shows up in server B"<<std::endl;
            sendToServer(mainServerSock, it->first.c_str(), 31994);
            std::cout<<"The Main Server has sent request for " <<in<< " to server B using UDP over port "<<MAIN_SERVER_PORT<<std::endl;
            memset(buffer, 0, sizeof(buffer));
            recvfrom(mainServerSock, buffer, sizeof(buffer), 0, NULL, NULL);
            std::cout<<"The Main server has received searching result(s) of "<<in<<" from Backend server B"<<std::endl;
            buffer[sizeof(buffer)] = '\0';
            printf("%s\n", buffer);
        }
        else if(it->second == 2){
            std::cout<<in<< " shows up in server C"<<std::endl;
            sendToServer(mainServerSock, it->first.c_str(), 32994);
            std::cout<<"The Main Server has sent request for " <<in<< " to server C using UDP over port "<<MAIN_SERVER_PORT<<std::endl;
            memset(buffer, 0, sizeof(buffer));
            recvfrom(mainServerSock, buffer, sizeof(buffer), 0, NULL, NULL);
            std::cout<<"The Main server has received searching result(s) of "<<in<<" from Backend server C"<<std::endl;
            buffer[sizeof(buffer)] = '\0';
            printf("%s\n", buffer);
        }
        }
        else{
            std::cout<<in<<" does not show up in Backend servers"<<std::endl;
        }

        std::cout<<"\n-----Start a new query-----\n";
        
    }

    close(mainServerSock);

    return 0;
}