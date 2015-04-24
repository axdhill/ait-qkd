/*
 * NetlinkNewSAMessage.h
 *
 * This class represents a message to add an IPsec SAD entry;
 * it takes its input and has to be delivered by a NetlinkManager
 * The class derives from the generic NetlinkXFRMMessage.
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

#ifndef NETLINKSAMESSAGE_H_
#define NETLINKSAMESSAGE_H_

#include "NLconstants.h"
#include "IPAddress.h"
#include "NetlinkMessage.h"
#include "CipherValidator.h"
#include "NetlinkSAConfig.h"

namespace qkd {

namespace IPsec {

class NetlinkNewSAMessage : public NetlinkMessage {
	public:
		NetlinkNewSAMessage(IPAddress source_add, IPAddress destination_add, NetlinkSAConfig config=NetlinkSAConfig());
		inline void setKey(char* key=NULL){
			if(key) memcpy(payload.key, key, payload.alg.alg_key_len);
		};
	private:
		xfrm_usersa_info 		xsinfo;
		class NewSAPayload{
		public:
			Rtattr rtattr;
			xfrm_algo alg;
			char key[MAX_KEY_SIZE];
		}payload;
		//int payload_size;
};

} /* namespace QuaKE */

} /* namespace qkd */

#endif /* NETLINKSAMESSAGE_H_ */
