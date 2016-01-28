/*
 * IPAddress.cpp
 *
 * Class to get IP addresses in Netlink-compatible format from a string
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

#include "IPAddress.h"
#include "Utils.h"
#include <sstream>  //Type stringstream
#include <string>	//Type string
#include <stdlib.h> //atoi()
#include <cstring>  //strcpy
#include <iostream> //cerr
#include <boost/regex.hpp>
#include <boost/asio/ip/address.hpp> //Only works with Boost linker option -lboost_system

using namespace qkd::IPsec;


IPAddress::IPAddress(std::string IP) {
	IPfamily=AF_UNSPEC;
	IPvalue={0};
	boost::asio::ip::address address;
	try{
	  address = address.from_string(IP);
	}
	catch(std::exception &e){
		throw IPAddressException("Invalid IP \""+IP+"\"; Boost says:"+e.what());
	}
	if(address.is_unspecified()) throw IPAddressException("Invalid IP \""+IP+"\"; unspecified address");
	else if(address.is_v4()){
		IPfamily=AF_INET;
		IPvalue.a4=Utils::letbe32(address.to_v4().to_ulong());
	}else if(address.is_v6()) {
		IPfamily=AF_INET6;
		boost::asio::ip::address_v6::bytes_type bytes;
		bytes=address.to_v6().to_bytes();
		for(int i=0; i<4; i++)
				for(int j=0; j<4; j++)
					//IPvalue.a6[i<1?i+2:i-2]=IPvalue.a6[i<1?i+2:i-2]|((int)bytes.at((4*i)+j)<<(j*8));
					IPvalue.a6[i]=IPvalue.a6[i]|((int)bytes.at((4*i)+j)<<(j*8));
	}else throw IPAddressException("Unknown Error.");
	if (IPfamily==AF_INET)	IPprefix=(uint8_t)32;
	if (IPfamily==AF_INET6)	IPprefix=(uint8_t)128;
	//********BOOST-FREE VERSION*****************
	/*uint32_t ip32=0;
	std::stringstream ss(IP);
	std::string temp;
	//For IPv4 addresses
	int ctr=0;
	while (getline(ss, temp, '.')) {
		  //if(!regex_match (temp,re)) throw IPAddressException("Invalid numbers in IP address "+IP);
		ip32 += (strtol(temp.c_str(),NULL,0)<<ctr);
		ctr+=8;
	}
	if(ctr != 32) throw IPAddressException("Wrong number of decimal groups for IP address "+IP);
	std::cout << "\n Trad. IP parse: " << ip32 << "\n";
	ret.a4=ip32;*/
	//*******************************************
}

IPAddress::IPAddress(std::string IP,int prefix):IPAddress(IP){
	if(prefix < 0) throw IPAddressException("Prefix can't be less than zero");
	if((IPfamily==AF_INET)  && (prefix > 32))  throw IPAddressException("IPv4 prefix can't be greater than 32");
	if((IPfamily==AF_INET6) && (prefix > 128)) throw IPAddressException("IPv6 prefix can't be greater than 128");
	IPprefix=(uint8_t)prefix;
}

IPAddress::IPAddress():IPAddress("0.0.0.0"){
	IPfamily=AF_INET; //only to suppress annoying Eclipse warnings
}

uint16_t IPAddress::getFamily(){
	return IPfamily;
}

xfrm_address_t IPAddress::getIPAddress(){
	return IPvalue;
}

uint8_t IPAddress::getPrefix(){
	return IPprefix;
}
