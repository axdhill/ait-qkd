/*
 * NetlinkNewSAMessage.cpp
 *
 * This class represents a message to add an IPsec SAD entry;
 * it takes its input and has to be delivered by a NetlinkManager.
 * The class derives from the generic NetlinkXFRM
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

#include "NetlinkNewSAMessage.h"
#include "NLconstants.h"
#include "Utils.h"
#include <string.h> //for memset(), strcpy()
#include <unistd.h> //for getpid()
#include <iostream> //for cin, std::cout, pthread_self()

using namespace qkd::IPsec;

NetlinkNewSAMessage::NetlinkNewSAMessage(IPAddress source_add, IPAddress destination_add, NetlinkSAConfig config){

	/*INIT*/
	CipherValidator val(config.getCipher());
	memset(&xsinfo,0,sizeof xsinfo);
	memset(&payload,0,sizeof payload);
	int payload_size=0;

	/*Fill XFRM structure */
	xsinfo.lft.soft_byte_limit   = config.getSoft_byte_limit();
	xsinfo.lft.hard_byte_limit   = config.getHard_byte_limit();
	xsinfo.lft.soft_packet_limit = config.getSoft_packet_limit();
	xsinfo.lft.hard_packet_limit = config.getHard_packet_limit();

	xsinfo.id.proto=IPPROTO_ESP;
	xsinfo.id.spi=config.getSpi();

	xsinfo.family=source_add.getFamily();
	if (xsinfo.family!=destination_add.getFamily()) throw IPAddressException("Source and destination address types do not match!");
	xsinfo.saddr=source_add.getIPAddress();
	xsinfo.id.daddr=destination_add.getIPAddress();
	xsinfo.sel.family=source_add.getFamily();
	xsinfo.sel.saddr=destination_add.getIPAddress();
	xsinfo.sel.daddr=source_add.getIPAddress();
	xsinfo.mode=XFRM_MODE_TRANSPORT;


	/*FILL payload structure*/
	//ciphers: aes, blowfish, des, rc2,
	strncpy(payload.alg.alg_name, config.getCipher().c_str(), sizeof payload.alg.alg_name);
	payload.alg.alg_key_len = val.getKeyLength();
	if(payload.alg.alg_key_len>MAX_KEY_SIZE) throw KeyException("Crypto algorithm unknown.");
	if(config.getKey()) memcpy((void*)payload.key, (void*)config.getKey(), payload.alg.alg_key_len);




	int size_unal = sizeof payload.rtattr + sizeof payload.alg + payload.alg.alg_key_len;
	payload_size = ((size_unal%4)==0)?size_unal:((size_unal/4)+1)*4;
	payload.rtattr.rta_len= payload_size; //MORE ACCURATE
	//payload.rtattr.rta_len= sizeof(payload); //EASIER

	payload.rtattr.rta_type= XFRMA_ALG_CRYPT;
	memcpy((void*)&payload,(void*)&payload,sizeof payload);

	/*SET NL header parameters*/
	setType(XFRM_MSG_NEWSA);
	setSize(NLMSG_LENGTH(sizeof xsinfo)+payload_size);
}

