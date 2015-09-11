/*
 * NetlinkDeleteSADcpp
 *
 * This class represents a message to delete an IPsec SAD entry;
 * it takes its input and has to be delivered by a NetlinkManager.
 * The class derives from the generic NetlinkXFRM
 *
 * Author: Stefan Marksteiner, <stefan.marksteiner@joanneum.at>
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

#include "NetlinkDeleteSAMessage.h"
#include "NLconstants.h"
#include "Utils.h"
#include <string.h> //for memset(), strcpy()
#include <unistd.h> //for getpid()
#include <iostream> //for cin, std::cout, pthread_self()

using namespace qkd::IPsec;

NetlinkDeleteSAMessage::NetlinkDeleteSAMessage(IPAddress source_add, IPAddress destination_add, NetlinkSAConfig config){

	/*INIT*/
	memset(&xsid,0,sizeof xsid);
	memset(&payload,0,sizeof payload);

	/*Fill XFRM structure */
	xsid.proto=IPPROTO_ESP;
	xsid.spi=config.getSpi();
	xsid.family=source_add.getFamily();
	if (xsid.family!=destination_add.getFamily()) throw IPAddressException("Source and destination address types do not match!");
	xsid.daddr=destination_add.getIPAddress();

	/*FILL payload structure*/
	payload.rtattr.rta_len=sizeof payload;
	payload.rtattr.rta_type=XFRMA_SRCADDR;
	payload.source=source_add.getIPAddress();

	/*SET NL header parameters*/
	setSize(NLMSG_LENGTH(sizeof(xsid)+sizeof(payload)));
	setType(XFRM_MSG_DELSA);
}


