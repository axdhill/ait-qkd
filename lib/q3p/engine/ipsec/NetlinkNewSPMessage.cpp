/*
 * NetlinkNewSPcpp
 *
 * This class represents a message to add an IPsec SPD entry;
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


#include "NetlinkNewSPMessage.h"
#include <string.h> //for memset()
#include <unistd.h> //for getpid()


using namespace qkd::IPsec;

NetlinkNewSPMessage::NetlinkNewSPMessage(IPAddress source_add, IPAddress destination_add, NetlinkSPConfig config) {

	/*INIT*/
	memset(&xpinfo,0,sizeof xpinfo);
	memset(&payload,0,sizeof payload);

	/*Fill XFRM structure */
	xpinfo.lft.soft_byte_limit = XFRM_INF;
	xpinfo.lft.hard_byte_limit = XFRM_INF;
	xpinfo.lft.soft_packet_limit = XFRM_INF;
	xpinfo.lft.hard_packet_limit = XFRM_INF;

	if(config.getOutbound())  xpinfo.dir = XFRM_POLICY_OUT;
	else xpinfo.dir = XFRM_POLICY_IN;
	xpinfo.sel.family=source_add.getFamily();
	if (xpinfo.sel.family!=destination_add.getFamily()) throw IPAddressException("Source and destination address types do not match!");
	xpinfo.sel.saddr=source_add.getIPAddress();
	xpinfo.sel.daddr=destination_add.getIPAddress();
	xpinfo.sel.prefixlen_s=source_add.getPrefix();
	xpinfo.sel.prefixlen_d=destination_add.getPrefix();
	xpinfo.sel.sport=config.getSource_port();
	xpinfo.sel.sport_mask=config.getSource_portmask();
	xpinfo.sel.dport=config.getDestination_port();
	xpinfo.sel.dport_mask=config.getDestination_portmask();


	/*FILL payload structure*/

	payload.rtattr.rta_len=sizeof (payload);
	payload.rtattr.rta_type= XFRMA_TMPL;
	payload.tmp.aalgos = (~(uint32_t)0);
	payload.tmp.ealgos = (~(uint32_t)0);
	payload.tmp.calgos = (~(uint32_t)0);
	payload.tmp.mode=XFRM_MODE_TRANSPORT;
	payload.tmp.id.proto=IPPROTO_ESP;
	payload.tmp.id.spi=config.getSpi();

	/*SET NL header parameters*/
	setType(XFRM_MSG_NEWPOLICY);	//Type: new SP
	setSize(NLMSG_LENGTH(sizeof(xpinfo))+sizeof(payload)); //complete packet size
}

NetlinkNewSPMessage::NetlinkNewSPMessage(IPAddress source_add, IPAddress destination_add, IPAddress tunnel_source_add, IPAddress tunnel_destination_add, NetlinkSPConfig config)
		:NetlinkNewSPMessage(source_add, destination_add, config){
	//***IF CONSTRUCTED WITH THIS CONSTRUCTOR THE SP SET AUTOMATICALLY FOR TUNNEL MODE***
	payload.tmp.mode=XFRM_MODE_TUNNEL;
	payload.tmp.saddr=tunnel_source_add.getIPAddress();
	payload.tmp.id.daddr=tunnel_destination_add.getIPAddress();
	payload.tmp.family =tunnel_source_add.getFamily();
	if (payload.tmp.family!=tunnel_destination_add.getFamily()) throw IPAddressException("Tunnel Source and destination address types do not match!");
	//**************************
}
