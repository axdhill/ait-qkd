/*
 * netlink_nlmsg.cpp
 * 
 * nlmsghdr wrapper
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

#include <linux/rtnetlink.h>

#include <qkd/common_macros.h>

#include "netlink_nlmsghdr.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// decl


/**
 * give a human readable string for a netlink flag bitmask
 * 
 * @param   nNetlinkFlags       the netlink flags
 * @param   bGetRequest         true for GET request flags
 * @param   bNewRequest         true for NEW request flags
 * @param   bDelRequest         true for DEL request flags
 * @return  string for the netlink flags
 */
static std::string nlmsghdr_flags_str(unsigned int nNetlinkFlags, bool bGetRequest, bool bNewRequest, bool bDelRequest);

    
/**
 * sets get, new and delete flag for netlink messages
 * 
 * @param   nNetlinkType        the netlink type
 * @param   bGetRequest         [out] type is a get request
 * @param   bNewRequest         [out] type is a new request
 * @param   bDelRequest         [out] type is a delete request
 */
static void nlmsghdr_type_flags(unsigned int nNetlinkType, bool & bGetRequest, bool & bNewRequest, bool & bDelRequest);


/**
 * give a human readable string for a netlink type
 * 
 * @param   nNetlinkType        the netlink type
 * @return  string for the netlink type
 */
static std::string nlmsghdr_type_str(unsigned int nNetlinkType);


// ------------------------------------------------------------
// code


/**
 * ctor
 */
netlink_nlmsghdr::netlink_nlmsghdr() : netlink_base() {
    reset();
}
    
    
/**
 * ctor
 * 
 * @param   cNetlinkMessage     the nlmsghdr kernel object to wrap
 */
netlink_nlmsghdr::netlink_nlmsghdr(nlmsghdr const & cNetlinkMessage) : netlink_base() {
    memcpy(&m_cNetlinkMessage, &cNetlinkMessage, sizeof(m_cNetlinkMessage));
}


/**
 * the name of the netlink kernel struct managed
 * 
 * @return  the name of the kernek struct managed
 */
std::string const & netlink_nlmsghdr::name() const { 
    static std::string const sName = "nlmsghdr"; 
    return sName;
}


/**
 * reset/init the netlink message to an empty (but valid) state
 */
void netlink_nlmsghdr::reset() {
    memset(&m_cNetlinkMessage, 0, sizeof(m_cNetlinkMessage));
}

    
/**
 * this method provides a JSON representation
 *
 * @return  a string holding a JSON of the current values
 */
std::string netlink_nlmsghdr::str() const {
    
    std::stringstream ss;
    
    bool bGetRequest = false;
    bool bNewRequest = false;
    bool bDelRequest = false;
    nlmsghdr_type_flags(m_cNetlinkMessage.nlmsg_type, bGetRequest, bNewRequest, bDelRequest);

    ss << "{ \"" << name() << "\": { ";
    ss << "\"nlmsg_len\": " << m_cNetlinkMessage.nlmsg_len << ", ";
    ss << "\"nlmsg_type\": \"" << nlmsghdr_type_str(m_cNetlinkMessage.nlmsg_type) << "\", ";
    ss << "\"nlmsg_flags\": \"" << nlmsghdr_flags_str(m_cNetlinkMessage.nlmsg_flags, bGetRequest, bNewRequest, bDelRequest) << "\", ";
    ss << "\"nlmsg_seq\": " << m_cNetlinkMessage.nlmsg_seq << ", ";
    ss << "\"nlmsg_pid\": " << m_cNetlinkMessage.nlmsg_pid;
    ss << " } }";
    
    return ss.str();
}


/**
 * give a human readable string for a netlink flag bitmask
 * 
 * @param   nNetlinkFlags       the netlink flags
 * @param   bGetRequest         true for GET request flags
 * @param   bNewRequest         true for NEW request flags
 * @param   bDelRequest         true for DEL request flags
 * @return  string for the netlink flags
 */
std::string nlmsghdr_flags_str(unsigned int nNetlinkFlags, bool bGetRequest, bool bNewRequest, UNUSED bool bDelRequest) {
    
    std::stringstream ss;
    bool bFirst = true;
    
    if (nNetlinkFlags & NLM_F_REQUEST) {
        if (!bFirst) ss << " | ";
        else bFirst = false;
        ss << "NLM_F_REQUEST";
    }
    
    if (nNetlinkFlags & NLM_F_MULTI) {
        if (!bFirst) ss << " | ";
        else bFirst = false;
        ss << "NLM_F_MULTI";
    }
    
    if (nNetlinkFlags & NLM_F_ACK) {
        if (!bFirst) ss << " | ";
        else bFirst = false;
        ss << "NLM_F_ACK";
    }
    
    if (nNetlinkFlags & NLM_F_ECHO) {
        if (!bFirst) ss << " | ";
        else bFirst = false;
        ss << "NLM_F_ECHO";
    }
    
    if (bGetRequest) {
    
        if (nNetlinkFlags & NLM_F_ROOT) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_ROOT";
        }
        
        if (nNetlinkFlags & NLM_F_MATCH) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_MATCH";
        }
        
        if (nNetlinkFlags & NLM_F_ATOMIC) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_ATOMIC";
        }
    }
    
    if (bNewRequest) {
    
        if (nNetlinkFlags & NLM_F_REPLACE) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_REPLACE";
        }
        
        if (nNetlinkFlags & NLM_F_EXCL) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_EXCL";
        }
        
        if (nNetlinkFlags & NLM_F_CREATE) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_CREATE";
        }
        
        if (nNetlinkFlags & NLM_F_APPEND) {
            if (!bFirst) ss << " | ";
            else bFirst = false;
            ss << "NLM_F_APPEND";
        }
    }
    
    return ss.str();
}


/**
 * sets get, new and delete flag for netlink messages
 * 
 * @param   nNetlinkType        the netlink type
 * @param   bGetRequest         [out] type is a get request
 * @param   bNewRequest         [out] type is a new request
 * @param   bDelRequest         [out] type is a delete request
 */
void nlmsghdr_type_flags(unsigned int nNetlinkType, bool & bGetRequest, bool & bNewRequest, bool & bDelRequest) {
    
    bGetRequest = false;
    bNewRequest = false;
    bDelRequest = false;
    
    switch (nNetlinkType) {
        
    case NLMSG_NOOP:
        break;
    case NLMSG_ERROR:
        break;
    case NLMSG_DONE:
        break;
    case RTM_NEWLINK:
        bNewRequest = true;
        break;
    case RTM_DELLINK:
        bDelRequest = true;
        break;
    case RTM_GETLINK:
        bGetRequest = true;
        break;
    case RTM_NEWADDR:
        bNewRequest = true;
        break;
    case RTM_DELADDR:
        bDelRequest = true;
        break;
    case RTM_GETADDR:
        bGetRequest = true;
        break;
    case RTM_NEWROUTE:
        bNewRequest = true;
        break;
    case RTM_DELROUTE:
        bDelRequest = true;
        break;
    case RTM_GETROUTE:
        bGetRequest = true;
        break;
    case RTM_NEWNEIGH:
        bNewRequest = true;
        break;
    case RTM_DELNEIGH:
        bDelRequest = true;
        break;
    case RTM_GETNEIGH:
        bGetRequest = true;
        break;
    case RTM_NEWRULE:
        bNewRequest = true;
        break;
    case RTM_DELRULE:
        bDelRequest = true;
        break;
    case RTM_GETRULE:
        bGetRequest = true;
        break;
    case RTM_NEWQDISC:
        bNewRequest = true;
        break;
    case RTM_DELQDISC:
        bDelRequest = true;
        break;
    case RTM_GETQDISC:
        bGetRequest = true;
        break;
    case RTM_NEWTCLASS:
        bNewRequest = true;
        break;
    case RTM_DELTCLASS:
        bDelRequest = true;
        break;
    case RTM_GETTCLASS:
        bGetRequest = true;
        break;
    case RTM_NEWTFILTER:
        bNewRequest = true;
        break;
    case RTM_DELTFILTER:
        bDelRequest = true;
        break;
    case RTM_GETTFILTER:
        bGetRequest = true;
        break;
    }
}


/**
 * give a human readable string for a netlink type
 * 
 * @param   nNetlinkType        the netlink type
 * @return  string for the netlink type
 */
std::string nlmsghdr_type_str(unsigned int nNetlinkType) {
    
    switch (nNetlinkType) {
        
    case NLMSG_NOOP:
        return "NLMSG_NOOP";
        break;
    case NLMSG_ERROR:
        return "NLMSG_ERROR";
        break;
    case NLMSG_DONE:
        return "NLMSG_DONE";
        break;
    case RTM_NEWLINK:
        return "RTM_NEWLINK";
        break;
    case RTM_DELLINK:
        return "RTM_DELLINK";
        break;
    case RTM_GETLINK:
        return "RTM_GETLINK";
        break;
    case RTM_NEWADDR:
        return "RTM_NEWADDR";
        break;
    case RTM_DELADDR:
        return "RTM_DELADDR";
        break;
    case RTM_GETADDR:
        return "RTM_GETADDR";
        break;
    case RTM_NEWROUTE:
        return "RTM_NEWROUTE";
        break;
    case RTM_DELROUTE:
        return "RTM_DELROUTE";
        break;
    case RTM_GETROUTE:
        return "RTM_GETROUTE";
        break;
    case RTM_NEWNEIGH:
        return "RTM_NEWNEIGH";
        break;
    case RTM_DELNEIGH:
        return "RTM_DELNEIGH";
        break;
    case RTM_GETNEIGH:
        return "RTM_GETNEIGH";
        break;
    case RTM_NEWRULE:
        return "RTM_NEWRULE";
        break;
    case RTM_DELRULE:
        return "RTM_DELRULE";
        break;
    case RTM_GETRULE:
        return "RTM_GETRULE";
        break;
    case RTM_NEWQDISC:
        return "RTM_NEWQDISC";
        break;
    case RTM_DELQDISC:
        return "RTM_DELQDISC";
        break;
    case RTM_GETQDISC:
        return "RTM_GETQDISC";
        break;
    case RTM_NEWTCLASS:
        return "RTM_NEWTCLASS";
        break;
    case RTM_DELTCLASS:
        return "RTM_DELTCLASS";
        break;
    case RTM_GETTCLASS:
        return "RTM_GETTCLASS";
        break;
    case RTM_NEWTFILTER:
        return "RTM_NEWTFILTER";
        break;
    case RTM_DELTFILTER:
        return "RTM_DELTFILTER";
        break;
    case RTM_GETTFILTER:
        return "RTM_GETTFILTER";
        break;
    }

    return "?unkown?";    
}

#endif
