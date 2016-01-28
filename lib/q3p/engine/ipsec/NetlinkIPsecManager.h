/*
 * NetlinkManager.h
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

#ifndef NETLINKIPSECMANAGER_H_
#define NETLINKIPSECMANAGER_H_

#include <string>
#include "KernelIPsecManager.h"
#include "NLconstants.h"
#include "NetlinkNewSAMessage.h"
#include "NetlinkDeleteSAMessage.h"
#include "NetlinkNewSPMessage.h"
#include "NetlinkDeleteSPMessage.h"
#include "IPAddress.h"
#include "NetlinkSPConfig.h"
#include <iostream>

namespace qkd {

namespace IPsec {

class NetlinkIPsecManager : public KernelIPsecManager{
public:
	//NetlinkManager();
	NetlinkIPsecManager(IPAddress source_add, IPAddress destination_add);
	virtual ~NetlinkIPsecManager();
	int addSP(NetlinkSPConfig config=NetlinkSPConfig());
	int addSP(IPAddress tunnel_source_add, IPAddress tunnel_destination_add, NetlinkSPConfig config=NetlinkSPConfig());
	int deleteSP();
	int addSA(NetlinkSAConfig config=NetlinkSAConfig());
	int deleteSA();
	inline void UpdateKey(char* key){
	  newSA.setKey(key);
	  //sendUrgentNLMessage(delSA);
	  sendUrgentNLMessage(newSA);
	}
	int receiveNLReturnMessage2();
private:
	void init();
	int netlinkSocket;
	sockaddr_nl s_nladdr, d_nladdr;
	IPAddress src, dst={0};
	NetlinkSPConfig SPparams;
	NetlinkSAConfig SAparams;
	msghdr msg;
	iovec iov;
	NetlinkNewSAMessage newSA;
	NetlinkDeleteSAMessage delSA;
	int sendNLMessage(NetlinkMessage &message);
	int receiveNLReturnMessage();
	inline void sendUrgentNLMessage(NetlinkMessage &message){
		iov.iov_base=message.getMessage();
		iov.iov_len=message.getSize();
		sendmsg(netlinkSocket, &msg, 0);
	}
};



} /* namespace QuaKE */

} /* namespace qkd */

#endif /* NETLINKIPSECMANAGER_H_ */
