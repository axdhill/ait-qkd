/*
 * netlink_ifinfomsg.cpp
 * 
 * ifinfomsg wrapper
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
#include <sstream>

#include <sys/socket.h>

#include "netlink_ifinfomsg.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// decl


/**
 * return the interface's change mask as string
 * 
 * @param   nChangeMask         the interface change mask value
 * @return  the interface change mask as string
 */
static std::string ifinfomsg_change_str(unsigned int nChangeMask);

    
/**
 * return the protocol/address familiy as string
 * 
 * @param   nFamiliy        the networking familiy
 * @return  the protocol/address familiy as human readable string
 */
static std::string ifinfomsg_familiy_str(unsigned char nFamiliy);


/**
 * return interface flags as string
 * 
 * @param   nFlags          the interface flags
 * @return  human readable flags
 */
static std::string ifinfomsg_flags_str(unsigned int nFlags);


// ------------------------------------------------------------
// code


/**
 * ctor
 */
netlink_ifinfomsg::netlink_ifinfomsg() : netlink_base() {
    reset();
}
    
    
/**
 * ctor
 * 
 * @param   cInterfaceInfo      the ifinfomsg kernel object to wrap
 */
netlink_ifinfomsg::netlink_ifinfomsg(ifinfomsg const & cInterfaceInfo) : netlink_base() {
    memcpy(&m_cInterfaceInfo, &cInterfaceInfo, sizeof(m_cInterfaceInfo));
}


/**
 * the name of the netlink kernel struct managed
 * 
 * @return  the name of the kernek struct managed
 */
std::string const & netlink_ifinfomsg::name() const { 
    static std::string const sName = "ifinfomsg"; 
    return sName;
}


/**
 * reset/init the netlink message to an empty (but valid) state
 */
void netlink_ifinfomsg::reset() {
    memset(&m_cInterfaceInfo, 0, sizeof(m_cInterfaceInfo));
}

    
/**
 * this method provides a JSON representation
 *
 * @return  a string holding a JSON of the current values
 */
std::string netlink_ifinfomsg::str() const {
    
    std::stringstream ss;
   
    ss << "{ \"" << name() << "\": { ";
    ss << "\"ifi_family\": " << ifinfomsg_familiy_str(m_cInterfaceInfo.ifi_family) << ", ";
    ss << "\"ifi_type\": " << (int)m_cInterfaceInfo.ifi_type << ", ";
    ss << "\"ifi_index\": " << m_cInterfaceInfo.ifi_index << ", ";
    ss << "\"ifi_flags\": " << ifinfomsg_flags_str(m_cInterfaceInfo.ifi_flags) << ", ";
    ss << "\"ifi_change\": " << ifinfomsg_change_str(m_cInterfaceInfo.ifi_change);
    ss << " } }";
    
    return ss.str();
}


/**
 * return the interface's change mask as string
 * 
 * @param   nChangeMask         the interface change mask value
 * @return  the interface change mask as string
 */
std::string ifinfomsg_change_str(unsigned int nChangeMask) {
    std::stringstream ss;
    ss << "\"0x" << std::hex << nChangeMask << std::dec << "\"";
    return ss.str();
}


/**
 * return the protocol/address familiy as string
 * 
 * @param   nFamiliy        the networking familiy
 * @return  the protocol/address familiy as human readable string
 */
std::string ifinfomsg_familiy_str(unsigned char nFamiliy) {
    
    std::stringstream ss;
    
    switch (nFamiliy) {
        
    case AF_UNSPEC:     ss << "\"AF_UNSPEC\"";      break;
    case AF_UNIX:       ss << "\"AF_UNIX | AF_FILE | AF_LOCAL\""; break;
    case AF_INET:       ss << "\"AF_INET\"";        break;
    case AF_AX25:       ss << "\"AF_AX25\"";        break;
    case AF_IPX:        ss << "\"AF_IPX\"";         break;
    case AF_APPLETALK:  ss << "\"AF_APPLETALK\"";   break;
    case AF_INET6:      ss << "\"AF_INET6\"";       break;
        
    default:
        ss << "\"?unknown familiy (" << (int)nFamiliy << ")?\"";
    }

    return ss.str();    
}


/**
 * return interface flags as string
 * 
 * @param   nFlags          the interface flags
 * @return  human readable flags
 */
std::string ifinfomsg_flags_str(unsigned int nFlags) {
    
    std::stringstream ss;
    ss << "\"0x" << std::hex << nFlags << std::dec << "\"";
    return ss.str();
}


#endif
