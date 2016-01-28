/*
 * NetlinkDeleteSADMEssage.h
 *
 * This class represents a message to delete an IPsec SAD entry;
 * it takes its input and has to be delivered by a NetlinkManager.
 * The class derives from the generic NetlinkXFRMMessage.
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

#ifndef NETLINKDELETESAMESSAGE_H_
#define NETLINKDELETESAMESSAGE_H_

#include "NLconstants.h"
#include "IPAddress.h"
#include "NetlinkMessage.h"
#include "NetlinkSAConfig.h"

namespace qkd {

namespace IPsec {

class NetlinkDeleteSAMessage  : public NetlinkMessage {
public:
	NetlinkDeleteSAMessage(IPAddress source_add, IPAddress destination_add, NetlinkSAConfig config=NetlinkSAConfig());
private:
	xfrm_usersa_id			xsid;
	class DeleteSAPayload{
	public:
		Rtattr rtattr;
		xfrm_address_t source;
	}payload;
};

} /* namespace QuaKE */

} /* namespace qkd */

#endif /* NETLINKDELETESAMESSAGE_H_ */
