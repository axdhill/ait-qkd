/*
 * NetlinkMessage.h
 *
 * This class is a virtual representation of an XFRM message;
 * it has to be instanciated with a concrete message.
 * The NetlinkManager uses this class and its subclasses to manage
 * the IPsec databases (SAD and SPD) via Netlink
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

#ifndef NETLINKMESSAGE_H_
#define NETLINKMESSAGE_H_

#include "NLconstants.h"
#include "IPAddress.h"
#include <iostream>
#include <string.h>
#include <unistd.h>


namespace qkd {

namespace IPsec {

class NetlinkMessage {
public:
	NetlinkMessage();
	virtual ~NetlinkMessage(){};
	virtual int getSize();
	virtual void* getMessage();
	virtual int getType();
	virtual void setUrgency(bool urgent);
protected:
	virtual void setSize(int size);
	virtual void setType(int type);
private:
	nlmsghdr nlhdr;
};

class Rtattr {
public:
	unsigned short  rta_len;
	unsigned short  rta_type;
};

} /* namespace QuaKE */

} /* namespace qkd */

#endif /* NETLINKMESSAGE_H_ */
