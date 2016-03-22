/*
 * route.cpp
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
 
 
// ------------------------------------------------------------
// incs

#include <string.h>

#include <boost/format.hpp>

#include "route.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// decl


/**
 * convert a inet4 address to string
 * 
 * @param   cAddress        pointer to address memory
 * @param   nHostLen        significant host bits
 * @param   bEmptyDrop      drop if address is 0
 * @return  a string represnting the address
 */
static std::string host_address(void const * cAddress, int nHostLen, bool bEmptyDrop);


// ------------------------------------------------------------
// code


/**
 * ctor
 */
route::route() : m_nDstHostLen(0), m_nSrcHostLen(0), m_nMetrics(0), m_nPriority(0), m_nInterface(0) {
    memset(&m_cDstAddress, 0, sizeof(m_cDstAddress));
    memset(&m_cSrcAddress, 0, sizeof(m_cSrcAddress));
    memset(&m_cGateway, 0, sizeof(m_cGateway));
}

    
/**
 * check if this is a empty route
 * 
 * @return  true, if  there are no values
 */
bool route::empty() const { 
    return ((m_cDstAddress.s_addr == 0) && (m_cSrcAddress.s_addr == 0) && (m_cGateway.s_addr == 0) && m_sInterface.empty()); 
}
   


/**
 * describe route as string
 * 
 * @return  a human readble string for the route
 */
std::string route::str() const {
    
    std::string sTo = host_address(&m_cDstAddress, m_nDstHostLen, false);
    std::string sFrom = host_address(&m_cSrcAddress, m_nSrcHostLen, false);
    std::string sGateway = host_address(&m_cGateway, m_nDstHostLen, true);
    
    boost::format cFormat("to: %-18s from: %-18s gw: %-18s dev: %8s (index: %2d) priority: %5d metrics: %5d");
    cFormat % sTo % sFrom % sGateway % m_sInterface % m_nInterface % m_nPriority % m_nMetrics;
    
    return cFormat.str();
}


/**
 * convert a inet4 address to string
 * 
 * @param   cAddress        pointer to address memory
 * @param   nHostLen        significant host bits
 * @param   bEmptyDrop      drop if address is 0
 * @return  a string represnting the address
 */
std::string host_address(void const * cAddress, int nHostLen, bool bEmptyDrop) {
    
    in_addr const * a = (in_addr const *)cAddress;
    if (a->s_addr == 0) {
        if (bEmptyDrop) {
            return "";
        }
        else {
            return "default";
        }
    }
    
    char cBuffer[256];
    inet_ntop(AF_INET, cAddress, cBuffer, 256);
    
    if ((nHostLen == 0) || (nHostLen == 32)) {
        return cBuffer;
    }
    
    boost::format cFormat = boost::format("%s/%u");
    cFormat % cBuffer % nHostLen;
    
    return cFormat.str();
}



#endif
