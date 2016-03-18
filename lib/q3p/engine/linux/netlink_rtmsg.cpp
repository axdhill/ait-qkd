/*
 * netlink_rtmsg.cpp
 * 
 * rtmsg wrapper
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
#include <sys/socket.h>

#include <sstream>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "netlink_rtmsg.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// decl


/**
 * return a string for an address family
 * 
 * @param   nAddressFamiliy         the address familiy asked
 * @return  a string holding known address families (or the value)
 */
static std::string rtmsg_rtm_family_str(char nAddressFamiliy);


/**
 * dump a the routing table messaga flags
 * 
 * @param   nRoutingMessageFlags    the routing message flags
 * @return  a string describing the routing message flags
 */
static std::string rtmsg_rtm_flags_str(unsigned char nRoutingMessageFlags);


/**
 * dump a the routing table message protocol
 * 
 * @param   nRoutingMessageProtocol     the routing message protocol
 * @return  a string describing the routing message protocol
 */
static std::string rtmsg_rtm_protocol_str(unsigned char nRoutingMessageProtocol);


/**
 * dump a the routing table message scope
 * 
 * @param   nRoutingMessageScope        the routing message scope
 * @return  a string describing the routing message scope
 */
static std::string rtmsg_rtm_scope_str(unsigned char nRoutingMessageScope);


/**
 * dump a the routing table message table
 * 
 * @param   nRoutingMessageTable        the routing message table
 * @return  a string describing the routing message table
 */
static std::string rtmsg_rtm_table_str(unsigned char nRoutingMessageTable);

    
/**
 * dump a the routing table message type
 * 
 * @param   nRoutingMessageType     the routing message type
 * @return  a string describing the routing message type
 */
static std::string rtmsg_rtm_type_str(unsigned char nRoutingMessageType);


// ------------------------------------------------------------
// code


/**
 * ctor
 */
netlink_rtmsg::netlink_rtmsg() : netlink_base() {
    reset();
}


/**
 * ctor
 * 
 * @param   cRoutingMessage             the kernel route attribute wrapped
 */
netlink_rtmsg::netlink_rtmsg(rtmsg const & cRoutingMessage) : netlink_base() {
    memcpy(&m_cRoutingMessage, &cRoutingMessage, sizeof(m_cRoutingMessage));
}


/**
 * the name of the netlink kernel struct managed
 * 
 * @return  the name of the kernek struct managed
 */
std::string const & netlink_rtmsg::name() const {
    static std::string const sName = "rtmsg"; 
    return sName;
}


/**
 * reset/init the netlink message to an empty (but valid) state
 */
void netlink_rtmsg::reset() {
    memset(&m_cRoutingMessage, 0, sizeof(m_cRoutingMessage));
}

    
/**
 * this method provides a JSON representation
 *
 * @return  a string holding a JSON of the current values
 */
std::string netlink_rtmsg::str() const {
    
    std::stringstream ss;
    
    ss << "{ \"" << name() << "\": { ";
    ss << "\"rtm_family\": " << rtmsg_rtm_family_str(m_cRoutingMessage.rtm_family) << ", ";
    ss << "\"rtm_dst_len\": " << (int)m_cRoutingMessage.rtm_dst_len << ", ";
    ss << "\"rtm_src_len\": " << (int)m_cRoutingMessage.rtm_src_len << ", ";
    ss << "\"rtm_tos\": " << (int)m_cRoutingMessage.rtm_tos << ", ";
    ss << "\"rtm_table\": " << rtmsg_rtm_table_str(m_cRoutingMessage.rtm_table) << ", ";
    ss << "\"rtm_protocol\": " << rtmsg_rtm_protocol_str(m_cRoutingMessage.rtm_protocol) << ", ";
    ss << "\"rtm_scope\": " << rtmsg_rtm_scope_str(m_cRoutingMessage.rtm_scope) << ", ";
    ss << "\"rtm_type\": " << rtmsg_rtm_type_str(m_cRoutingMessage.rtm_type) << ", ";
    ss << "\"rtm_flags\": " << rtmsg_rtm_flags_str(m_cRoutingMessage.rtm_flags);
    ss << " } }";
    
    return ss.str();
}


/**
 * return a string for an address family
 * 
 * @param   nAddressFamiliy         the address familiy asked
 * @return  a string holding known address families (or the value)
 */
std::string rtmsg_rtm_family_str(char nAddressFamiliy) {
    
    std::stringstream ss;
    
    switch (nAddressFamiliy) {
        
    case AF_UNSPEC:     ss << "\"AF_UNSPEC\"";      break;
    case AF_UNIX:       ss << "\"AF_UNIX | AF_FILE | AF_LOCAL\""; break;
    case AF_INET:       ss << "\"AF_INET\"";        break;
    case AF_AX25:       ss << "\"AF_AX25\"";        break;
    case AF_IPX:        ss << "\"AF_IPX\"";         break;
    case AF_APPLETALK:  ss << "\"AF_APPLETALK\"";   break;
    case AF_INET6:      ss << "\"AF_INET6\"";       break;
    default:
        ss << (int)nAddressFamiliy;
    }

    return ss.str();    
}


/**
 * dump a the routing table messaga flags
 * 
 * @param   nRoutingMessageFlags    the routing message flags
 * @return  a string describing the routing message flags
 */
std::string rtmsg_rtm_flags_str(unsigned char nRoutingMessageFlags) {
    
    std::stringstream ss;
    
    bool bFirst = true;
    
    ss << "\"";
    
    if (nRoutingMessageFlags & RTM_F_NOTIFY) {
        if (!bFirst) ss << "| ";
        else bFirst = false;
        ss << "RTM_F_NOTIFY ";
    }
    if (nRoutingMessageFlags & RTM_F_CLONED) {
        if (!bFirst) ss << "| ";
        else bFirst = false;
        ss << "RTM_F_CLONED ";
    }
    if (nRoutingMessageFlags & RTM_F_EQUALIZE) {
        if (!bFirst) ss << " | ";
        else bFirst = false;
        ss << "RTM_F_EQUALIZE ";
    }
    
    ss << "(0x" << std::hex << (int)nRoutingMessageFlags << std::dec << ")\"";
    
    return ss.str();
}


/**
 * dump a the routing table message protocol
 * 
 * @param   nRoutingMessageProtocol     the routing message protocol
 * @return  a string describing the routing message protocol
 */
std::string rtmsg_rtm_protocol_str(unsigned char nRoutingMessageProtocol) {

    std::stringstream ss;
    
    switch (nRoutingMessageProtocol) {
        
    case RTPROT_UNSPEC:     ss << "\"RTPROT_UNSPEC\""; break;
    case RTPROT_REDIRECT:   ss << "\"RTPROT_REDIRECT\""; break;
    case RTPROT_KERNEL:     ss << "\"RTPROT_KERNEL\""; break;
    case RTPROT_BOOT:       ss << "\"RTPROT_BOOT\""; break;
    case RTPROT_STATIC:     ss << "\"RTPROT_STATIC\""; break;
    default:
        ss << "\"?unkown routing protocol? (" << (int)nRoutingMessageProtocol << ")\"";
    }
    
    return ss.str();
}


/**
 * dump a the routing table message scope
 * 
 * @param   nRoutingMessageScope        the routing message scope
 * @return  a string describing the routing message scope
 */
std::string rtmsg_rtm_scope_str(unsigned char nRoutingMessageScope) {
    
    std::stringstream ss;
    
    switch (nRoutingMessageScope) {
        
    case RT_SCOPE_UNIVERSE: ss << "\"RT_SCOPE_UNIVERSE\""; break;
    case RT_SCOPE_SITE:     ss << "\"RT_SCOPE_SITE\""; break;
    case RT_SCOPE_LINK:     ss << "\"RT_SCOPE_LINK\""; break;
    case RT_SCOPE_HOST:     ss << "\"RT_SCOPE_HOST\""; break;
    case RT_SCOPE_NOWHERE:  ss << "\"RT_SCOPE_NOWHERE\""; break;
    default:
        ss << "\"?unkown routing scope? (" << (int)nRoutingMessageScope << ")\"";
    }
    
    return ss.str();
}


/**
 * dump a the routing table message table
 * 
 * @param   nRoutingMessageTable        the routing message table
 * @return  a string describing the routing message table
 */
std::string rtmsg_rtm_table_str(unsigned char nRoutingMessageTable) {
    
    std::stringstream ss;

    switch (nRoutingMessageTable) {
        
    case RT_TABLE_UNSPEC:        ss << "\"RT_TABLE_UNSPEC\""; break;
    case RT_TABLE_DEFAULT:       ss << "\"RT_TABLE_DEFAULT\""; break;
    case RT_TABLE_MAIN:         ss << "\"RT_TABLE_MAIN\""; break;
    case RT_TABLE_LOCAL:     ss << "\"RT_TABLE_LOCAL\""; break;
    default:
        ss << "\"?unkown routing table?(" << (int)nRoutingMessageTable << ")\"";
    }
    
    return ss.str();
}

    
/**
 * dump a the routing table message type
 * 
 * @param   nRoutingMessageType     the routing message type
 * @return  a string describing the routing message type
 */
std::string rtmsg_rtm_type_str(unsigned char nRoutingMessageType) {
    
    std::stringstream ss;

    switch (nRoutingMessageType) {
        
    case RTN_UNSPEC:        ss << "\"RTN_UNSPEC\""; break;
    case RTN_UNICAST:       ss << "\"RTN_UNICAST\""; break;
    case RTN_LOCAL:         ss << "\"RTN_LOCAL\""; break;
    case RTN_BROADCAST:     ss << "\"RTN_BROADCAST\""; break;
    case RTN_ANYCAST:       ss << "\"RTN_ANYCAST\""; break;
    case RTN_MULTICAST:     ss << "\"RTN_MULTICAST\""; break;
    case RTN_BLACKHOLE:     ss << "\"RTN_BLACKHOLE\""; break;
    case RTN_UNREACHABLE:   ss << "\"RTN_UNREACHABLE\""; break;
    case RTN_PROHIBIT:      ss << "\"RTN_PROHIBIT\""; break;
    case RTN_THROW:         ss << "\"RTN_THROW\""; break;
    case RTN_NAT:           ss << "\"RTN_NAT\""; break;
    case RTN_XRESOLVE:      ss << "\"RTN_XRESOLVE\""; break;
    default:
        ss << "\"?unkown routing type?(" << (int)nRoutingMessageType << ")\"";
    }
    
    return ss.str();
}

   
#endif
