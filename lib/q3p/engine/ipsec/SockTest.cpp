/*
 * SockTest.cpp
 *
 *  Created on: 12.10.2013
 *      Author: mrs
 */

#include "SockTest.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
using namespace std;

SockTest::SockTest(string hostname, int port){
	struct sockaddr_in serv_addr;
    struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) std::cout << "ERROR opening socket";

    server = gethostbyname(hostname.c_str());
    if (server == NULL) std::cout << "Server ERROR";

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            std::cout << "ERROR connecting"; else cout << "SUCCESS";
}

string SockTest::readSocket(){
	char* buffer= new char[256];
	bzero(buffer, 256);
	read(sockfd,buffer,255);
	return string(buffer);
}

void SockTest::writeSocket(string data){
	write(sockfd,data.c_str(),data.length());
}

SockTest::~SockTest() {
    close(sockfd);
	std::cout << "Socket destroyed\n";
}

