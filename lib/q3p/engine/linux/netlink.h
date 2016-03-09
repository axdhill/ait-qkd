/*
 * netlink.h
 * 
 * interface to Linux kernel netlink
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
 
 
#ifndef __QKD_Q3P_LINUX_NETLINK_H_
#define __QKD_Q3P_LINUX_NETLINK_H_

// ------------------------------------------------------------
// incs

#include <arpa/inet.h>
#include <sys/types.h>
#include <linux/netlink.h>

#include <map>
#include <string>

// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

   
/**
 * This is a more convenient interface to the linux netlink API
 */
class netlink {
    

public:
    
    
    /**
     * a single route
     */
    struct route {
        
        struct in_addr cDstAddress;             /**< TO: destination address */
        struct in_addr cSrcAddress;             /**< FROM: source address */
        struct in_addr cGateway;                /**< gateway address to use */
        std::string sInterface;                 /**< interface name */
        
        
        /**
         * describe route as string
         * 
         * @return  a human readble string for the route
         */
        std::string str() const;
    };
    
    
    /**
     * a routing table indexed by "TO: destination address"
     */
    typedef std::map<in_addr, route> routing_table;
    
    
    /**
     * get the current kernel main routing table
     * 
     * @return  the current routing table
     */
    routing_table get_routing_table();
    
        
    /**
     * get the netlink singelton
     * 
     * @return  the netlink singelton instance
     */
    static netlink & instance();


private:
    
    
    /**
     * ctor
     */
    netlink();
    
    
    /**
     * dtor
     */
    ~netlink();
    
    
    /**
     * receive from the netlink layer
     * 
     * The buffer has to be allocated prior with a proper buffer size. On success the buffer
     * will be filled with nlmsghdr structs linear. On success the number of bytes written
     * to the buffer is returned, else -1.
     * 
     * @param   nSocket             the netlink socket to use
     * @param   cBuffer             the buffer which receives the netlink messages
     * @param   nMessageNumber      the message number to receive
     * @return  number of bytes received (-1 in case of error)
     */
    int recv(int nSocket, char * cBuffer, ssize_t nBufferSize, unsigned int nMessageNumber);
    
        
    /**
     * send a netlink message to the kernel
     * 
     * @param   nSocket             the netlink socket to use
     * @param   cNetlinkMessage     the message to be sent
     * @return  message number sent (or 0 in case or error)
     */
    unsigned int send(int nSocket, struct nlmsghdr * cNetlinkMessage);


    /**
     * the netlink route socket
     */
    int m_nNetlinkRouteSocket;
    
};
  

}

}


#endif

#endif
