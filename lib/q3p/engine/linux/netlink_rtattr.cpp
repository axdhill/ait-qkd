/*
 * netlink_rtattr.cpp
 * 
 * rtattr wrapper
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

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <qkd/utility/memory.h>

#include "netlink_rtattr.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// decl


/**
 * the routing attribute type as a string
 * 
 * @param   nNetlinkMessageType     the netlink message type
 * @param   nRoutingAttributeType   the routing attribute type
 * @return  a string describing the routing attribute type
 */
static std::string rtattr_type_str(uint64_t nNetlinkMessageType, uint64_t nRoutingAttributeType);


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   nNetlinkMessageType         the associated netlink message type
 * @param   nValueLen                   number of additional bytes for the value
 */
netlink_rtattr::netlink_rtattr(uint64_t nNetlinkMessageType, uint64_t nValueLen) : netlink_base(), m_nNetlinkMessageType(nNetlinkMessageType) {
    m_cRouteAttribute = std::shared_ptr<void>(new char(RTA_LENGTH(nValueLen)));
    (*this)->rta_len = RTA_LENGTH(nValueLen);
}

    
/**
 * ctor
 * 
 * @param   nNetlinkMessageType         the associated netlink message type
 * @param   cRouteAttribute             the kernel route attribute wrapped
 */
netlink_rtattr::netlink_rtattr(uint64_t nNetlinkMessageType, rtattr const & cRouteAttribute) : netlink_base(), m_nNetlinkMessageType(nNetlinkMessageType) {
    m_cRouteAttribute = std::shared_ptr<void>(new char(cRouteAttribute.rta_len));
    memcpy(m_cRouteAttribute.get(), &cRouteAttribute, cRouteAttribute.rta_len);
}


/**
 * the name of the netlink kernel struct managed
 * 
 * @return  the name of the kernek struct managed
 */
std::string const & netlink_rtattr::name() const {
    static std::string const sName = "rtattr"; 
    return sName;
}


/**
 * reset/init the netlink message to an empty (but valid) state
 */
void netlink_rtattr::reset() {
    m_cRouteAttribute = nullptr;
}

    
/**
 * size of the netlink kernel object managed
 * 
 * @return  size in bytes of the object returned by data()
 */
uint64_t netlink_rtattr::size() const {
    if (!m_cRouteAttribute) return 0;
    return ((rtattr const *)data())->rta_len;
}

    
/**
 * this method provides a JSON representation
 *
 * @return  a string holding a JSON of the current values
 */
std::string netlink_rtattr::str() const {
    
    std::stringstream ss;
    
    ss << "{ \"" << name() << "\": ";

    rtattr const * cRouteAttribute = (rtattr const *)data();
    if (!cRouteAttribute) {
        ss << "null";
    }
    else {
        ss << "{ ";
        ss << "\"rta_len\": " << cRouteAttribute->rta_len << ", ";
        ss << "\"rta_type\": " << rtattr_type_str(nlmsghdr_type(), cRouteAttribute->rta_type) << ", ";
        ss << "\"value size\": " << value_size() << ", ";
        ss << "\"value\": ";
        if (value() == nullptr) {
            ss << "null";
        }
        else {
            ss << "\"hex: " << qkd::utility::memory::wrap((qkd::utility::memory::value_t *)value(), value_size()).as_hex() << "\"";
        }
        ss << "}";
    }
    
    ss << " }";
    
    return ss.str();
}


/**
 * return the value part of the attribute
 * 
 * @return  the value part of the attribute
 */
void * netlink_rtattr::value() {
    
    rtattr const * cRouteAttribute = (rtattr const *)data();
    if (!cRouteAttribute) return nullptr;
    if (cRouteAttribute->rta_len == sizeof(rtattr)) return nullptr;
    
    return RTA_DATA((rtattr *)data());
}


/**
 * return the value part of the attribute
 * 
 * @return  the value part of the attribute
 */
void const * netlink_rtattr::value() const {
    if (!data()) return nullptr;
    return RTA_DATA((rtattr *)data());
}


/**
 * return the value part of the attribute
 * 
 * @return  the value part of the attribute
 */
uint64_t netlink_rtattr::value_size() const {
    
    rtattr const * cRouteAttribute = (rtattr const *)data();
    if (!cRouteAttribute) return 0;
    
    void const * cValue = value();
    return (cRouteAttribute->rta_len - ((char const *)cValue - (char const *)cRouteAttribute));
}


/**
 * the routing attribute type as a string
 * 
 * @param   nNetlinkMessageType     the netlink message type
 * @param   nRoutingAttributeType   the routing attribute type
 * @return  a string describing the routing attribute type
 */
std::string rtattr_type_str(uint64_t nNetlinkMessageType, uint64_t nRoutingAttributeType) {
    
    std::stringstream ss;
    
    switch (nNetlinkMessageType) {
        
    case RTM_NEWROUTE:
    case RTM_DELROUTE:
    case RTM_GETROUTE:
        
        switch (nRoutingAttributeType) {
            
        case RTA_UNSPEC:    ss << "\"RTA_UNSPEC\""; break;
        case RTA_DST:       ss << "\"RTA_DST\""; break;
        case RTA_SRC:       ss << "\"RTA_SRC\""; break;
        case RTA_IIF:       ss << "\"RTA_IIF\""; break;
        case RTA_OIF:       ss << "\"RTA_OIF\""; break;
        case RTA_GATEWAY:   ss << "\"RTA_GATEWAY\""; break;
        case RTA_PRIORITY:  ss << "\"RTA_PRIORITY\""; break;
        case RTA_PREFSRC:   ss << "\"RTA_PREFSRC\""; break;
        case RTA_METRICS:   ss << "\"RTA_METRICS\""; break;
        case RTA_MULTIPATH: ss << "\"RTA_MULTIPATH\""; break;
        case RTA_PROTOINFO: ss << "\"RTA_PROTOINFO\""; break;
        case RTA_FLOW:      ss << "\"RTA_FLOW\""; break;
        case RTA_CACHEINFO: ss << "\"RTA_CACHEINFO\""; break;
        case RTA_TABLE:     ss << "\"RTA_TABLE\""; break;
        case RTA_MARK:      ss << "\"RTA_MARK\""; break;
        case RTA_MFC_STATS: ss << "\"RTA_MFC_STATS\""; break;
        case RTA_VIA:       ss << "\"RTA_VIA\""; break;
        case RTA_NEWDST:    ss << "\"RTA_NEWDST\""; break;
        case RTA_PREF:      ss << "\"RTA_PREF\""; break;
        default:
            ss << "\"?unkown routing attribute type? (" << nRoutingAttributeType << ") for RTM_NEWROUTE | RTM_DELROUTE | RTM_GETROUTE\"";
        }
        break;
        
    default:
        ss << "\"?unkown routing attribute type? (" << (int)nRoutingAttributeType << ")\"";
    }
    
    return ss.str();
}

    
#endif
