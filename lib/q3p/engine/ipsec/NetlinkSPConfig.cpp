/*
 * SPconfig.cpp
 *
 *  Created on: 17.11.2013
 *      Author: mrs
 */

#include "NetlinkSPConfig.h"
#include "Utils.h"

using namespace qkd::IPsec;

NetlinkSPConfig::NetlinkSPConfig(bool outbound, unsigned spi, unsigned source_port, unsigned destination_port, unsigned source_portmask, unsigned destination_portmask)
:outbound(outbound), spi(spi), source_port(source_port), destination_port(destination_port), source_portmask(source_portmask), destination_portmask(destination_portmask){}

uint16_t NetlinkSPConfig::getSource_port()
{
	return Utils::letbe16((uint16_t)source_port);
}

uint16_t NetlinkSPConfig::getDestination_port()
{
	return Utils::letbe16((uint16_t)destination_port);
}

uint16_t NetlinkSPConfig::getSource_portmask()
{
	return Utils::letbe16((uint16_t)source_portmask);
}

uint16_t NetlinkSPConfig::getDestination_portmask()
{
	return Utils::letbe16((uint16_t)destination_portmask);
}

bool NetlinkSPConfig::getOutbound(){
	return outbound;
}

uint32_t  NetlinkSPConfig::getSpi(){
	return Utils::letbe32((uint32_t)spi);
}
