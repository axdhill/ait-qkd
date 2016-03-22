/*
 * netlink_message.h
 * 
 * a single netlink message to be sent or received
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
 
#ifndef __QKD_Q3P_LINUX_NETLINK_MESSAGE_H_
#define __QKD_Q3P_LINUX_NETLINK_MESSAGE_H_


// ------------------------------------------------------------
// incs

#include <list>
#include <memory>
#include <string>

#include "netlink_base.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

   
/**
 * This class holds a whole netlink message to be sent and received
 */
class netlink_message : public std::list<std::shared_ptr<qkd::q3p::netlink_base>> {

public:

    
    /**
     * insert a new message object at end
     * 
     * @param   cNetlinkMessageObject       the object to add at end
     */
    void add(netlink_base const & cNetlinkMessageObject) { push_back(cNetlinkMessageObject.clone()); }
    
    
    /**
     * returns the error code if this is received reply
     * 
     * The returned error code is negativ as in nlmsgerr.
     * 0 ... means: ACK
     * 1 ... means: this is not an error message 
     * 
     * @return  error code of the NLMSG_ERROR message
     */
    int error() const;
    

    /**
     * turn the whole netlink message into a JSON string
     *
     * @return  the netlink message as JSON string
     */
    std::string str() const;

};



}

}


#endif

#endif
