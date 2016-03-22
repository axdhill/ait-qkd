/*
 * route.h
 * 
 * a single route
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
 
 
#ifndef __QKD_Q3P_LINUX_ROUTE_H_
#define __QKD_Q3P_LINUX_ROUTE_H_


// ------------------------------------------------------------
// incs

#include <arpa/inet.h>

#include <string>
#include <vector>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

   
/**
 * This represents a single route
 */
class route {
    

public:
        
    in_addr m_cDstAddress;          /**< TO: destination address */
    in_addr m_cSrcAddress;          /**< FROM: source address */
    in_addr m_cGateway;             /**< gateway address to use */
    
    int m_nDstHostLen;              /**< number of significant bits of the destination address */
    int m_nSrcHostLen;              /**< number of significant bits of the source address */
    int m_nMetrics;                 /**< route metric */
    int m_nPriority;                /**< priority of route */
    
    int m_nInterface;               /**< interface index */
    std::string m_sInterface;       /**< interface name */
    

    /**
     * ctor
     */
    route();
    
        
    /**
     * check if this is a empty route
     * 
     * @return  true, if  there are no values
     */
    bool empty() const;
    
    
    /**
     * describe route as string
     * 
     * @return  a human readble string for the route
     */
    std::string str() const;
};
    
    
/**
 * a routing table
 */
typedef std::vector<route> routing_table;


}

}

    
#endif

#endif
