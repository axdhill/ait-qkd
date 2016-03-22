/*
 * netlink_parser_error.cpp
 * 
 * parse NEW_ROUTE, DEL_ROUTE, GET_ROUTE answers from kernel
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
 * AIT Austrian Institute of Technology GmbH
 * Donau-City-Strasse 1 | 1220 Vienna | Austria
 * http://www.ait.ac.at
 *
 * This file is part of the AIT QKD Software Suite.
 *
 * The AIT QKD Software Suite is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 * 
 * The AIT QKD Software Suite is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with the AIT QKD Software Suite. 
 * If not, see <http://www.gnu.org/licenses/>.
 */


// this is only Linux code
#if defined(__linux__)
 

// ------------------------------------------------------------
// incs

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <qkd/utility/debug.h>
#include <qkd/common_macros.h>

#include "netlink.h"
#include "netlink_nlmsgerr.h"
#include "netlink_nlmsghdr.h"
#include "netlink_parser_error.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * parse the message stored in cBuffer and add result to cMessage
 * 
 * @param   cMessage            message object to be filled
 * @param   cBuffer             memory returned from the kernel
 * @param   nSize               size of memory blob returned from the kernel
 * @return  true, if succefully parsed
 */
bool netlink_parser_error::parse(netlink_message & cMessage, char * cBuffer, uint32_t nSize) {
    
    // parse an error information from the kernel
    
    if (nSize < sizeof(nlmsghdr)) {
        qkd::utility::debug(netlink::debug()) << "size of kernel answer too small to parse";
        return false;
    }
    
    nlmsghdr * cNlMsgHdr = (nlmsghdr *)cBuffer;
    if (cNlMsgHdr->nlmsg_type != NLMSG_ERROR) {
        throw std::runtime_error("wrong parser instance for kernel message chosen");
    }
    
    netlink_nlmsghdr cNetlinkMessage(*cNlMsgHdr);
    cMessage.add(cNetlinkMessage);
    
    nlmsgerr * cNlMsgErr = (nlmsgerr *)NLMSG_DATA(cNlMsgHdr);
    cMessage.add(netlink_nlmsgerr(*cNlMsgErr));
    
    return true;
}


#endif
