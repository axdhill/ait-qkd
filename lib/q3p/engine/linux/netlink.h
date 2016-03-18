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
     * a single netlink socket
     */
    struct socket {
        int nSocket;                        /**< socket identifier */
        __u32 nSequenceNumber;              /**< message sequence number */
    };
    
    
    /**
     * a single route
     */
    struct route {
        
        in_addr cDstAddress;            /**< TO: destination address */
        in_addr cSrcAddress;            /**< FROM: source address */
        in_addr cGateway;               /**< gateway address to use */
        std::string sInterface;         /**< interface name */
        
        
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
     * add a route to the kernel routing table
     * 
     * @param   cRoute      the route to add
     * @return  true on success
     */
    bool add_route(route const & cRoute);
    
    
    /**
     * return debug flag
     * 
     * @return  return the debug flag
     */
    static bool & debug() { return m_bDebug; }
    
    
    /**
     * return debug kernel message blobs flag
     * 
     * @return  return the debug kernel message blobs flag
     */
    static bool & debug_message_blobs() { return m_bDebugMessageBlobs; }
    
    
    /**
     * remove a route from the kernel routing table
     * 
     * @param   cRoute      the route to remove
     * @return  true on success
     */
    bool del_route(route const & cRoute);
    
    
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
     * debug netlink send/recv
     */
    static bool m_bDebug;
    
    
    /**
     * debug netlink kernel blobs send/recv
     */
    static bool m_bDebugMessageBlobs;
    
    
    /**
     * the rtnetlink sockket
     */
    qkd::q3p::netlink::socket m_cNetlinkRouteSocket;
    
};
  

}

}


#endif

#endif
