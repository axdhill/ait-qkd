/*
 * netlink_parser.cpp
 * 
 * parse a netlink kernel answer
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

#include "netlink_parser.h"
#include "netlink_parser_error.h"
#include "netlink_parser_xroute.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * get a proper parser instance for a certain netlink message type
 * 
 * @param   nNetlinkMessageType     the nlmsghdr type of the message returned
 * @return  instance to parser object
 */
std::shared_ptr<netlink_parser> netlink_parser::create(unsigned int nNetlinkMessageType) {
    
    switch (nNetlinkMessageType) {
        
    case NLMSG_ERROR:
        return std::shared_ptr<netlink_parser>(new netlink_parser_error);
        
    case RTM_NEWROUTE:
    case RTM_DELROUTE:
    case RTM_GETROUTE:
        return std::shared_ptr<netlink_parser>(new netlink_parser_xroute);
        
    }
    
    throw std::runtime_error("unknown netlink message type to parse");
}


#endif
