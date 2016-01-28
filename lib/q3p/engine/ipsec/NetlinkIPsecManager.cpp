/*
 * NetlinkIPsecManager.cpp
 *
 * Handler class for dealing with the Netlink/XFRM engine to manage the
 * IPsec SP and SA databases
 *
 * Author: Stefan Marksteiner, <stefan.marksteiner@joanneum.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
 * AIT Austrian Institute of Technology GmbH
 * Donau-City-Strasse 1 | 1220 Vienna | Austria
 * http://www.ait.ac.at
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


//TODO: Design question @Oliver: SPI or source address as identifier?
//TODO: Design question @Oliver: separate data structure or memcpy?

#include "NetlinkIPsecManager.h"
#include "NetlinkNewSAMessage.h"
#include "NetlinkNewSPMessage.h"
#include "NetlinkDeleteSPMessage.h"
#include "NetlinkDeleteSAMessage.h"
#include "IPAddress.h" //for IPv4fromString()
#include "NetlinkSPConfig.h"
#include "Exception.h"
#include "Utils.h"
#include <sys/socket.h> //for socket(), bind(),sendmsg() and close() and constants like AF_INET, AF_NETLINK and SOCK_RAW
#include <linux/netlink.h> //for NLMSG_DATA(), types sockaddr_nl, nlmsghdr  and constants like NETLINK_XFRM
#include <linux/xfrm.h> //for constant XFRM_MSG_NEWPOLICY and type xfrm_userpolicy_info
#include <iostream> //for cin, std::cout, pthread_self()
#include <unistd.h> //for getpid(),close()
#include <string.h> //for memset(), strcpy()
#include <stdlib.h> //for malloc()
#include <cstdint> //for uint8_t

using namespace qkd::IPsec;


//TODO: write inline documentation
/**
 * accept a key for processing
 *
 * each time a key is ought to be processed by a module, this
 * method is called. if this method returns false the key is
 * discarded
 *
 * The default implementation discards DISCLOSED keys.
 *
 * @param   cKey            the key to check
 * @return  true, if the key should be processed by this module
 */

NetlinkIPsecManager::NetlinkIPsecManager(IPAddress source_add, IPAddress destination_add)
:src(source_add), dst(destination_add), newSA(src,dst), delSA(src,dst){

	memset(&SPparams,0,sizeof SPparams);
	//params.source_prefix=src.getPrefix();
	//params.destination_prefix=dst.getPrefix();
	netlinkSocket=socket(AF_NETLINK ,SOCK_RAW , NETLINK_XFRM );

	/* source address */
	memset(&s_nladdr, 0 ,sizeof(s_nladdr)); //Initalize, includes to fill padding with zeroes
	s_nladdr.nl_family= AF_NETLINK ;
	s_nladdr.nl_pid = pthread_self() << 16 | getpid();  //from this actual thread in this program
	bind(netlinkSocket, (struct sockaddr*)&s_nladdr, sizeof(s_nladdr));

	/* destination address */
	memset(&d_nladdr, 0 ,sizeof(d_nladdr)); //Initalize, includes to fill padding with zeroes
	d_nladdr.nl_family= AF_NETLINK ;
	d_nladdr.nl_pid = 0; // destined to kernel

	/* msg */
	memset(&msg,0,sizeof(msg));
	msg.msg_name = (void *) &d_nladdr ;
	msg.msg_namelen=sizeof(d_nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	newSA.setUrgency(true);
	delSA.setUrgency(true);
	//std::cout << "\n***NetlinkConnection established***\n";
}

NetlinkIPsecManager::~NetlinkIPsecManager() {
	//deleteSA(); //TODO: if exists() delete SA
	close(netlinkSocket);
	//std::cout << "\n***NetlinkConnection terminated***\n";
}

int NetlinkIPsecManager::addSP(NetlinkSPConfig config){
	SPparams = config;
	NetlinkNewSPMessage add(src,dst,SPparams);
	return sendNLMessage(add);
}

int NetlinkIPsecManager::addSP(IPAddress tunnel_source_add, IPAddress tunnel_destination_add, NetlinkSPConfig config){
	SPparams = config;
	NetlinkNewSPMessage add(src,dst,tunnel_source_add,tunnel_destination_add,SPparams);
	return sendNLMessage(add);
}

int NetlinkIPsecManager::deleteSP(){
	NetlinkDeleteSPMessage del(src,dst,SPparams);
	return sendNLMessage(del);
}

int NetlinkIPsecManager::addSA(NetlinkSAConfig config){
	SAparams = config;
	NetlinkNewSAMessage add(src,dst, SAparams);
	newSA.setKey(config.getKey());
	return sendNLMessage(add);
}

int NetlinkIPsecManager::deleteSA(){
	NetlinkDeleteSAMessage del(src,dst,SAparams);
	return sendNLMessage(del);
}

int NetlinkIPsecManager::sendNLMessage(NetlinkMessage &message){
	try
	{
		iov.iov_base=message.getMessage();
		iov.iov_len=message.getSize();
		if (sendmsg(netlinkSocket, &msg, 0)<0) throw NLSendException("Unknown Error");
		receiveNLReturnMessage();
	}
	catch (NLSendException &e){
		std::cerr << "Error on sending Netlink message: \n";
		exit(EXIT_FAILURE);
	}
	catch (std::bad_alloc &e){
		std::cerr << "Error on memory allocation: " << e.what() << "\n";
		exit(EXIT_FAILURE);
	}
	catch (std::exception &e){
		std::cerr << "Error: " << e.what() << "\n";
		exit(EXIT_FAILURE);
	}
	return 0;
}

int NetlinkIPsecManager::receiveNLReturnMessage(){
			 char buf[16384];
			 memset(buf,0,sizeof(buf));
			 iov.iov_base = buf;
			 iov.iov_len = sizeof(buf);
			 recvmsg(netlinkSocket, &msg, 0);
			 //nlmsghdr* answer = (struct nlmsghdr*)buf;
			 char *bufp = buf;
			 bufp+=16;
			 struct nlmsgerr *err = (nlmsgerr*)bufp;
			 try{ if(err->error) throw NLReturnException(-err->error);}
			 catch(NLReturnException &err){ std::cerr << err.getMessage();};
	return 0;
}


int NetlinkIPsecManager::receiveNLReturnMessage2(){
			 char buf[16384];
			 memset(buf,0,sizeof(buf));
			 iov.iov_base = buf;
			 iov.iov_len = sizeof(buf);
			 recvmsg(netlinkSocket, &msg, 0);
			 //nlmsghdr* answer = (struct nlmsghdr*)buf;
			 char *bufp = buf;
			 bufp+=16;
			 struct nlmsgerr *err = (nlmsgerr*)bufp;
			 std::cout<< "\n" << err->error << "\n";

	return 0;
}


