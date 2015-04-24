/*
 * SockTest.h
 *
 *  Created on: 12.10.2013
 *      Author: mrs
 */

#ifndef SOCKTEST_H_
#define SOCKTEST_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
using namespace std;


class SockTest {
public:
	SockTest(string hostname="localhost", int port=22);
	virtual ~SockTest();
	string readSocket();
	void writeSocket(string data);
private:
	int sockfd;
};

#endif /* SOCKTEST_H_ */
