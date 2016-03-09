/*
 * netlink.cpp
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
 

/*
 * Netlink interaction inspired by 
 * http://stackoverflow.com/questions/3288065/getting-gateway-to-use-for-a-given-ip-in-ansi-c#3288983
 */


// ------------------------------------------------------------
// incs


#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

// ait
#include <qkd/utility/debug.h>
#include "netlink.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// vars


/**
 * netlink message sequence number
 */
static unsigned int g_nNetlinkMessageNumber = 0;


// ------------------------------------------------------------
// code


/**
 * describe route as string
 * 
 * @return  a human readble string for the route
 */
std::string netlink::route::str() const {
    
    // TODO
    return "TBD";
}


/**
 * ctor
 */
netlink::netlink() {
    m_nNetlinkSocket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (m_nNetlinkSocket == -1) {
        qkd::utility::debug() << "Failed to create netlink socket. Error: " << strerror(errno);
    }
}


/**
 * dtor
 */
netlink::~netlink() {
    if (m_nNetlinkSocket == -1) {
        close(m_nNetlinkSocket);
    }
}


/**
 * get the current kernel main routing table
 * 
 * @return  the current routing table
 */
netlink::routing_table netlink::get_routing_table() {
    
    netlink::routing_table res;
    
    // TODO
    
    return res;
}


/**
 * get the netlink singelton
 * 
 * @return  the netlink singelton instance
 */
netlink & netlink::instance() {
    static netlink cNetlink;
    return cNetlink;
}


/**
 * receive from the netlink layer
 * 
 * The buffer has to be allocated prior with a proper buffer size. On success the buffer
 * will be filled with nlmsghdr structs linearly. On success the number of bytes written
 * to the buffer is returned, else -1.
 * 
 * @param   cBuffer             the buffer which receives the netlink messages
 * @param   nMessageNumber      the message number to receive
 * @return  number of bytes received (-1 in case of error)
 */
int netlink::recv(char * cBuffer, ssize_t nBufferSize, unsigned int nMessageNumber) {
    
    if (m_nNetlinkSocket == -1) {
        qkd::utility::debug() << "Refused to receive netlink message on invalid socket.";
        return 0;
    }
    
    unsigned int nProcessId = getpid();
    int nRead = 0;
    int nMessageLen = 0;

    struct nlmsghdr * cNetlinkMessage = nullptr;
    
    do {
        
        nRead = ::recv(m_nNetlinkSocket, cBuffer, nBufferSize - nMessageLen, 0);
        if (nRead < 0) {
            qkd::utility::debug() << "Failed to read from netlink socket. Error: " << strerror(errno);
            return -1;
        }

        // TODO: check if the message number and the destination pid is ok
        
        cNetlinkMessage = (struct nlmsghdr *)cBuffer;

        if ((NLMSG_OK(cNetlinkMessage, nRead) == 0) || (cNetlinkMessage->nlmsg_type == NLMSG_ERROR)) {
            qkd::utility::debug() << "Error in received netlink message. Error: " << strerror(errno);
            return -1;
        }

        if (cNetlinkMessage->nlmsg_type == NLMSG_DONE) {
            break;
        } 
        else {
            cBuffer += nRead;
            nMessageLen += nRead;
        }

        if ((cNetlinkMessage->nlmsg_flags & NLM_F_MULTI) == 0) {
            // no multipart message --> no further recv necessary
            break;
        }
        
    } while ((cNetlinkMessage->nlmsg_seq != nMessageNumber) || (cNetlinkMessage->nlmsg_pid != nProcessId));
    
    return nMessageLen;
}


/**
 * send a netlink message to the kernel
 * 
 * @param   cNetlinkMessage     the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
unsigned int netlink::send(struct nlmsghdr * cNetlinkMessage) {
    
    if (m_nNetlinkSocket == -1) {
        qkd::utility::debug() << "Refused to send netlink message on invalid socket.";
        return 0;
    }
    if (cNetlinkMessage == nullptr) {
        qkd::utility::debug() << "Refused to send NULL netlink message.";
        return 0;
    }
    
    ++g_nNetlinkMessageNumber;
    if (!g_nNetlinkMessageNumber) ++g_nNetlinkMessageNumber;
    
    cNetlinkMessage->nlmsg_seq = g_nNetlinkMessageNumber;
    cNetlinkMessage->nlmsg_pid = getpid();
    
    if (::send(m_nNetlinkSocket, cNetlinkMessage, cNetlinkMessage->nlmsg_len, 0) < 0) {
        qkd::utility::debug() << "Failed to send netlink message. Error: " << strerror(errno);
        return 0;
    }
    
    return g_nNetlinkMessageNumber;
}


#endif
