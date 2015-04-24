/*
 * NetlinkDeleteSPMessage.cpp
 *
 *  Created on: 15.11.2013
 *      Author: mrs
 */

#include "NetlinkDeleteSPMessage.h"
#include <string.h> //for memset(), strcpy()

using namespace qkd::IPsec;

NetlinkDeleteSPMessage::NetlinkDeleteSPMessage(IPAddress source_add, IPAddress destination_add, NetlinkSPConfig config) {

	memset(&xpid,0,sizeof xpid);

	if(config.getOutbound())  xpid.dir = XFRM_POLICY_OUT;
	else xpid.dir = XFRM_POLICY_IN;
	xpid.sel.family=source_add.getFamily();
	if (xpid.sel.family!=destination_add.getFamily()) throw IPAddressException("Source and destination address types do not match!");
	xpid.sel.saddr=source_add.getIPAddress();
	xpid.sel.daddr=destination_add.getIPAddress();
	xpid.sel.prefixlen_s=source_add.getPrefix();
	xpid.sel.prefixlen_d=destination_add.getPrefix();
	xpid.sel.sport=config.getSource_port();
	xpid.sel.sport_mask=config.getSource_portmask();
	xpid.sel.dport=config.getDestination_port();
	xpid.sel.dport_mask=config.getDestination_portmask();


	setSize(NLMSG_LENGTH(sizeof(xpid)));
	setType(XFRM_MSG_DELPOLICY);

}
