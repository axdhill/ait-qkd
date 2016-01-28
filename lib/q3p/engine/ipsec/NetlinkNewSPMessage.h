/*
 * NetlinkNewSPMessage.h
 *
 * This class represents a message to add an IPsec SPD entry;
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

#ifndef NETLINKNEWSPMESSAGE_H_
#define NETLINKNEWSPMESSAGE_H_

#include "NLconstants.h"
#include "NetlinkMessage.h"
#include "NetlinkSPConfig.h"

namespace qkd {

namespace IPsec {

class NetlinkNewSPMessage : public NetlinkMessage {
	public:
		NetlinkNewSPMessage(IPAddress source_add, IPAddress destination_add, NetlinkSPConfig config=NetlinkSPConfig());


		NetlinkNewSPMessage(IPAddress source_add, IPAddress destination_add, IPAddress tunnel_source_add, IPAddress tunnel_destination_add, NetlinkSPConfig config=NetlinkSPConfig());

	private:
		xfrm_userpolicy_info	xpinfo;
		class NewSPPayload{
		public:
			Rtattr rtattr;
			xfrm_user_tmpl tmp;
		}payload;
};

} /* namespace QuaKE */

} /* namespace qkd */

#endif /* NETLINKNEWSPMESSAGE_H_ */
