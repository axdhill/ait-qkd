/*
 * IPAddress.h
 *
 * Class to get IP addresses in Netlink-compatible format from a string
 *
 * Autor: Stefan Marksteiner, <stefan.marksteiner@joanneum.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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

#include "NLconstants.h"
#include "Exception.h"
#include <string>
#include <array>


#ifndef IPADDRESS_H_
#define IPADDRESS_H_

namespace qkd {

namespace IPsec {


class IPAddress {
public:
	IPAddress(std::string IP);
	IPAddress(std::string IP,int prefix);
	IPAddress();
	xfrm_address_t getIPAddress();
	uint16_t getFamily();
	uint8_t getPrefix();
private:
	xfrm_address_t IPvalue;
	uint16_t IPfamily;
	uint8_t IPprefix;
};



} /* namespace QuaKE */

} /* namespace qkd */

#endif /* IPADDRESS_H_ */
