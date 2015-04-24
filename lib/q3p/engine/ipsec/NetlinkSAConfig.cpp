/*
 * NetlinkSAConfig.cpp
 *
 *  Created on: 01.12.2013
 *      Author: mrs
 */

#include "NetlinkSAConfig.h"
#include "Utils.h"

qkd::IPsec::NetlinkSAConfig::NetlinkSAConfig(char* key, std::string cipher, unsigned int spi
		,unsigned long long hard_byte_limit,unsigned long long soft_byte_limit, unsigned long long hard_packet_limit,unsigned long long soft_packet_limit)
:key(key), cipher(cipher), spi(spi)
 ,hard_byte_limit(hard_byte_limit),soft_byte_limit(soft_byte_limit),hard_packet_limit(hard_packet_limit),soft_packet_limit(soft_packet_limit)
{

}


char* qkd::IPsec::NetlinkSAConfig::getKey(){
	return key;
}

std::string qkd::IPsec::NetlinkSAConfig::getCipher(){
	return cipher;
}

uint32_t qkd::IPsec::NetlinkSAConfig::getSpi(){
	return Utils::letbe32(spi);
}

uint64_t qkd::IPsec::NetlinkSAConfig::getSoft_byte_limit(){
	return soft_byte_limit;
}

uint64_t qkd::IPsec::NetlinkSAConfig::getHard_byte_limit(){
	return hard_byte_limit;
}

uint64_t qkd::IPsec::NetlinkSAConfig::getSoft_packet_limit(){
	return soft_packet_limit;
}

uint64_t qkd::IPsec::NetlinkSAConfig::getHard_packet_limit(){
	return hard_packet_limit;
}
