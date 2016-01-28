/*
 * NetlinkMessage.cpp
 *
 * This class is a virtual representation of an XFRM message;
 * it has to be instanciated with a concrete message.
 * The NetlinkManager uses this class and its subclasses to manage
 * the IPsec databases (SAD and SPD) via Netlink
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

#include "NetlinkMessage.h"


using namespace qkd::IPsec;

NetlinkMessage::NetlinkMessage(){
	memset(&nlhdr,0,sizeof nlhdr);
	nlhdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;//|NLM_F_CREATE|NLM_F_EXCL;
	nlhdr.nlmsg_pid=pthread_self() << 16 | getpid();
	nlhdr.nlmsg_seq=0;
}

void NetlinkMessage::setUrgency(bool urgent){
	if(urgent) nlhdr.nlmsg_flags=NLM_F_REQUEST;
	else nlhdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
}


int NetlinkMessage::getSize(){
		return nlhdr.nlmsg_len;
}

void* NetlinkMessage::getMessage() {
	return (void *)&nlhdr;
}

int NetlinkMessage::getType(){
	return nlhdr.nlmsg_type;
}

void NetlinkMessage::setSize(int size){
	nlhdr.nlmsg_len=size;
}

void NetlinkMessage::setType(int type){
	nlhdr.nlmsg_type=type;
}

